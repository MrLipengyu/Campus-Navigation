#include "NavigationController.h"
#include "../core/pathfinding/Dijkstra.h"
#include <QDebug>

namespace controller {

NavigationController::NavigationController(const core::CampusMap& campusMap, graphics::MapView& mapView, QObject* parent)
    : QObject(parent), m_campusMap(campusMap), m_mapView(mapView) {
    // 🌟 核心绑定：当地图报告“跑完了”或“被打断了”，控制器立刻清理状态机！
    connect(&m_mapView, &graphics::MapView::autoNavigationFinished,
            this, &NavigationController::onNavigationFinished);
}

// 接收到设置起点的指令
void NavigationController::setStartNode(int buildingId) {
    m_startBuildingId = buildingId;
    qDebug() << "[Controller] 起点已更新为 ID:" << buildingId;
    tryPlanRoute(); // 每次状态改变，尝试寻路
}

// 接收到设置终点的指令
void NavigationController::setEndNode(int buildingId) {
    m_endBuildingId = buildingId;
    qDebug() << "[Controller] 终点已更新为 ID:" << buildingId;
    tryPlanRoute(); // 每次状态改变，尝试寻路
}

// 尝试规划路径（状态检查）
void NavigationController::tryPlanRoute() {
    // 如果起点和终点都已经设置
    if (m_startBuildingId != -1 && m_endBuildingId != -1) {

        // 防止起终点相同
        if (m_startBuildingId == m_endBuildingId) {
            qDebug() << "[Controller] 起点与终点相同，已清空路径。";
            m_mapView.clearPath();
            return;
        }

        // 状态合法，开始实际寻路
        planAndDrawRoute();
    } else {
        // 状态不全，清空已有路径
        m_mapView.clearPath();
    }
}

// 实际执行 Dijkstra 的核心函数
void NavigationController::planAndDrawRoute() {
    const core::Building* startB = m_campusMap.getBuilding(m_startBuildingId);
    const core::Building* endB = m_campusMap.getBuilding(m_endBuildingId);

    if (!startB || !endB) return;

    qDebug() << "[Controller] 调用 Dijkstra 开始寻路...";

    core::Pathfinder pathfinder(m_campusMap.getGraph());
    std::vector<int> path = pathfinder.findShortestPath(startB->entrance_node_id, endB->entrance_node_id);

    if (path.empty()) {
        qDebug() << "[Controller] 警告：没有找到可达的路径！";
        m_mapView.clearPath(); // 如果找不到路，确保线被清空
    } else {
        qDebug() << "[Controller] 寻路成功！命令 UI 画线。";
        m_mapView.drawPath(path);
    }

    if (path.empty()) {
        qDebug() << "[Controller] 警告：没有找到可达的路径！";
        m_mapView.clearPath();
    } else {
        qDebug() << "[Controller] 寻路成功！命令 UI 画线，并启动自动导航。";
        m_mapView.drawPath(path);

        // 👇 新增：画完线后，立即把路径丢给地图，启动自动驾驶！
        m_mapView.startAutoNavigation(path);
    }
}

// 👇 实现状态重置槽函数
void NavigationController::onNavigationFinished() {
    qDebug() << "[Controller] 收到导航结束指令，重置状态机。";

    // 1. 清空大脑里的起终点记忆
    m_startBuildingId = -1;
    m_endBuildingId = -1;

    // 2. 清除地图上残留的红线 (既然走到了，导航线就该消失了)
    m_mapView.clearPath();

    // 3. 发送信号，命令 MainWindow 清空右侧面板的起终点文字
    emit navigationStateReset();
}

} // namespace controller