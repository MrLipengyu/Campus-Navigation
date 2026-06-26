#include "NavigationController.h"
#include "../core/pathfinding/Dijkstra.h"
#include <QDebug>

namespace controller {

NavigationController::NavigationController(const core::CampusMap& campusMap, graphics::MapView& mapView, QObject* parent)
    : QObject(parent), m_campusMap(campusMap), m_mapView(mapView) {

    connect(&m_mapView, &graphics::MapView::autoNavigationFinished,
            this, &NavigationController::onNavigationFinished);
}

void NavigationController::setStartNode(int buildingId) {
    m_startBuildingId = buildingId;
    qDebug() << "[Controller] 起点已更新为 ID:" << buildingId;
}

void NavigationController::addDestNode(int buildingId) {
    // 检查重复：起点不能是目的地，也不能重复添加相同的目的地
    if (m_startBuildingId == buildingId) return;
    for (int id : m_destBuildingIds) {
        if (id == buildingId) return;
    }

    m_destBuildingIds.push_back(buildingId);
    qDebug() << "[Controller] 成功添加途经点，当前总计:" << m_destBuildingIds.size();
}

void NavigationController::clearRoute() {
    m_startBuildingId = -1;
    m_destBuildingIds.clear();
    m_mapView.clearPath();
    m_mapView.stopAutoNavigation();
    emit navigationStateReset();
}

void NavigationController::startMultiNavigation() {
    if (m_startBuildingId == -1 || m_destBuildingIds.empty()) {
        qDebug() << "[Controller] 警告：条件不足，无法发车！";
        return;
    }

    const core::Building* startB = m_campusMap.getBuilding(m_startBuildingId);
    if (!startB) return;

    // 将建筑 ID 转化为路网节点 ID
    std::vector<int> destNodeIds;
    for (int bId : m_destBuildingIds) {
        const core::Building* b = m_campusMap.getBuilding(bId);
        if (b) destNodeIds.push_back(b->entrance_node_id);
    }

    core::Pathfinder pathfinder(m_campusMap.getGraph());
    std::vector<int> visitOrderNodes; // 算法吐出的真实访问顺序

    // 🌟 调用多目标 TSP 算法
    std::vector<int> path = pathfinder.findTSPPath(startB->entrance_node_id, destNodeIds, visitOrderNodes);

    if (path.empty()) {
        qDebug() << "[Controller] 错误：无法找到连通路径！";
        m_mapView.clearPath();
    } else {
        // 构建用于 UI 显示的访问顺序字符串
        QString orderText = "<b>最优访问顺序:</b><br>";
        for (size_t i = 0; i < visitOrderNodes.size(); ++i) {
            // 通过 entrance_node_id 反查建筑名称
            for (const auto& [id, b] : m_campusMap.getAllBuildings()) {
                if (b.entrance_node_id == visitOrderNodes[i]) {
                    orderText += QString::fromStdString(b.name);
                    if (i != visitOrderNodes.size() - 1) orderText += " <span style='color:red;'>➡</span> ";
                    break;
                }
            }
        }

        emit routeOrderUpdated(orderText); // 通知界面更新文字

        // 让地图画线并开始自动导航
        m_mapView.drawPath(path);
        m_mapView.startAutoNavigation(path);
    }
}

void NavigationController::onNavigationFinished() {
    clearRoute(); // 导航结束后自动清理战场
}

} // namespace controller