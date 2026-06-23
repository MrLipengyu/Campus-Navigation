#include "CampusMap.h"

#include <iostream>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <cmath> // 用于 std::hypot 计算距离

namespace core {

bool CampusMap::loadFromJson(const QString& filePath) {
    QFile file(filePath);
    //只读/文本模式
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开地图文件:" << filePath;
        return false;
    }

    //读取文件所有字节，并返回一个二进制字节数组
    QByteArray jsonData = file.readAll();
    file.close();

    //json文件解析(返回qt可操作的对象oc)，并将解误信息存储在parseError中
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    //错误判断
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误:" << parseError.errorString();
        return false;
    }

    //取出根json对象
    QJsonObject rootObj = doc.object();
    m_graph.clear();
    m_buildings.clear();

    // 1. 解析 Nodes（先加载节点，才能算边的距离）
    QJsonArray nodesArray = rootObj["nodes"].toArray();
    for (const QJsonValue& val : nodesArray) {
        QJsonObject nodeObj = val.toObject();
        int id = nodeObj["id"].toInt();
        int x = nodeObj["x"].toInt();
        int y = nodeObj["y"].toInt();
        m_graph.addNode(id, x, y);
    }

    // 2. 解析 Edges 并自动计算物理距离（权重）
    QJsonArray edgesArray = rootObj["edges"].toArray();
    for (const QJsonValue& val : edgesArray) {
        QJsonObject edgeObj = val.toObject();
        int fromId = edgeObj["from"].toInt();
        int toId = edgeObj["to"].toInt();

        const Node* n1 = m_graph.getNode(fromId);
        const Node* n2 = m_graph.getNode(toId);

        if (n1 && n2) {
            // 利用勾股定理自动计算两点间的像素距离作为权重
            double distance = std::hypot(n1->x - n2->x, n1->y - n2->y);
            m_graph.addUndirectedEdge(fromId, toId, distance);
        } else {
            qWarning() << "警告: 发现无效的边连接 (" << fromId << "-" << toId << ")";
        }
    }

    // 3. 解析 Buildings（业务数据）
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

        m_buildings[b.id] = b;
    }

    std::cout << "地图加载成功！共加载"
              << m_buildings.size() << "栋建筑,"
              << m_graph.getAllNodes().size() << "个节点.\n";
    return true;
}

const Building* CampusMap::getBuilding(int id) const {
    auto it = m_buildings.find(id);
    if (it != m_buildings.end()) {
        return &(it->second);
    }
    return nullptr;
}

const std::unordered_map<int, Building>& CampusMap::getAllBuildings() const {
    return m_buildings;
}

} // namespace core