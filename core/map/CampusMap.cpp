#include "CampusMap.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <cmath> // 引入 cmath 用于 std::hypot 计算两点间的欧几里得距离

namespace core {

// ==============================================================================
// 核心加载函数：从 JSON 文件读取静态路网，并对接 SQLite 数据库加载动态业务数据
// ==============================================================================
bool CampusMap::loadFromJson(const QString& filePath) {
    // 1. 打开 JSON 地图文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[数据层] 无法打开地图文件:" << filePath;
        return false;
    }

    // 2. 读取并解析 JSON 数据
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[数据层] JSON 解析错误:" << parseError.errorString();
        return false;
    }

    QJsonObject rootObj = doc.object();

    // 清空现有的图数据和建筑数据，准备重新加载
    m_graph.clear();
    m_buildings.clear();

    // ================== 第一部分：加载底层的纯路网（图结构）==================

    // 3. 解析 Nodes（必须先加载节点，才能在解析边的时候计算距离）
    QJsonArray nodesArray = rootObj["nodes"].toArray();
    for (const QJsonValue& val : nodesArray) {
        QJsonObject nodeObj = val.toObject();
        int id = nodeObj["id"].toInt();
        int x = nodeObj["x"].toInt();
        int y = nodeObj["y"].toInt();

        // 将纯净的路径节点加入图中（不携带建筑业务数据）
        // 如果你的 Graph::addNode 依然要求传 name 和 info，请写为 m_graph.addNode(id, "", "", x, y);
        // 根据我们之前的重构，这里应该是纯坐标传入：
        m_graph.addNode(id, x, y);
    }

    // 4. 解析 Edges（连线）并自动计算物理距离（权重）
    QJsonArray edgesArray = rootObj["edges"].toArray();
    for (const QJsonValue& val : edgesArray) {
        QJsonObject edgeObj = val.toObject();
        int fromId = edgeObj["from"].toInt();
        int toId = edgeObj["to"].toInt();

        // 尝试从图中获取这两个节点，以获取它们的 x, y 坐标
        const Node* n1 = m_graph.getNode(fromId);
        const Node* n2 = m_graph.getNode(toId);

        if (n1 && n2) {
            // 【高级技巧】利用勾股定理 (hypot) 自动计算两点间的像素距离作为寻路权重
            double distance = std::hypot(n1->x - n2->x, n1->y - n2->y);
            m_graph.addUndirectedEdge(fromId, toId, distance);
        } else {
            qWarning() << "[数据层] 警告: 发现无效的边连接 (" << fromId << "-" << toId << ")，请检查 JSON";
        }
    }

    // ================== 第二部分：加载上层的业务数据（建筑信息）==================

    // 5. 初始化并连接 SQLite 数据库
    if (!m_dbManager.initialize()) {
        qWarning() << "[数据层] 数据库初始化失败，将无法进行数据持久化！";
    }

    // 6. 尝试从数据库获取所有已存储的建筑数据
    std::vector<Building> dbBuildings = m_dbManager.getAllBuildings();

    // 7. 判断数据库状态，决定数据来源（JSON or SQLite）
    if (dbBuildings.empty()) {
        // 【情况 A】：数据库为空（通常是首次运行程序）
        qDebug() << "[数据层] 数据库为空，开始从 JSON 导入基础建筑数据到 SQLite...";

        QJsonArray buildingsArray = rootObj["buildings"].toArray();
        for (const QJsonValue& val : buildingsArray) {
            QJsonObject bObj = val.toObject();
            Building b;
            b.id = bObj["id"].toInt();
            b.name = bObj["name"].toString().toStdString();
            b.info = bObj["info"].toString().toStdString();
            b.ui_x = bObj["ui_x"].toInt();
            b.ui_y = bObj["ui_y"].toInt();
            b.hitbox_radius = bObj["hitbox_radius"].toInt();
            b.entrance_node_id = bObj["entrance_node_id"].toInt();

            // 存入内存中的字典，供快速查询
            m_buildings[b.id] = b;

            // 【核心】写入 SQLite 数据库进行持久化！
            m_dbManager.insertBuilding(b);
        }
        qDebug() << "[数据层] 成功将" << m_buildings.size() << "栋建筑信息初始化至数据库。";

    } else {
        // 【情况 B】：数据库里已有数据（后续运行程序）
        // 优先使用数据库里的数据，因为这些数据可能被管理员（用户）修改过！
        qDebug() << "[数据层] 检测到 SQLite 数据库，从数据库加载建筑业务数据...";

        for (const auto& b : dbBuildings) {
            // 直接将数据库读出的模型存入内存字典
            m_buildings[b.id] = b;
        }
    }

    qDebug() << "[数据层] 地图整体加载完毕！共"
             << m_buildings.size() << "栋建筑,"
             << m_graph.getAllNodes().size() << "个路网节点.";

    return true;
}

// ==============================================================================
// 业务查询接口：根据 ID 获取对应的建筑数据常量指针
// ==============================================================================
const Building* CampusMap::getBuilding(int id) const {
    auto it = m_buildings.find(id);
    if (it != m_buildings.end()) {
        return &(it->second);
    }
    return nullptr; // 没找到则返回空指针
}

// ==============================================================================
// 业务查询接口：获取所有建筑数据（供 MapView 渲染标签时遍历使用）
// ==============================================================================
const std::unordered_map<int, Building>& CampusMap::getAllBuildings() const {
    return m_buildings;
}

// ==============================================================================
// 业务修改接口：管理员模式修改建筑信息，并同时同步给数据库和内存
// ==============================================================================
bool CampusMap::updateBuildingInfo(int id, const std::string& newInfo) {
    // 1. 先尝试向底层 SQLite 数据库发起更新请求
    if (m_dbManager.updateBuildingInfo(id, newInfo)) {
        // 2. 如果数据库更新成功，同步修改当前内存中的缓存数据
        // 这样就不需要重新加载整个地图，UI 能够瞬间读取到新数据
        m_buildings[id].info = newInfo;
        qDebug() << "[数据层] 建筑 ID:" << id << "的信息更新成功，且已持久化至数据库。";
        return true;
    }

    // 如果数据库更新失败（可能是磁盘权限或连接断开等异常）
    qWarning() << "[数据层] 建筑 ID:" << id << "的信息更新失败！";
    return false;
}

} // namespace core