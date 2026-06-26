#pragma once
#include <QObject>
#include "../core/map/CampusMap.h"
#include "../graphics/MapView.h"

namespace controller {

class NavigationController : public QObject {
    Q_OBJECT

public:
    NavigationController(const core::CampusMap& campusMap, graphics::MapView& mapView, QObject* parent = nullptr);

    // 👇 1. 新增：通知主窗口去更新 UI 标签的信号
signals:
    void navigationStateReset();

    // 👇 新增：当计算出新路径时，告诉 UI 实际的访问顺序
    void routeOrderUpdated(const QString& orderText);

public slots:
    // 明确的显式指令槽函数
    void setStartNode(int buildingId);

    // 👇 修改槽函数：支持添加多个目的地和控制导航
    void addDestNode(int buildingId);
    void clearRoute();
    void startMultiNavigation();
    void onNavigationFinished();
private:
    // 检查状态并尝试寻路
    void tryPlanRoute();
    void planAndDrawRoute();

private:
    const core::CampusMap& m_campusMap;
    graphics::MapView& m_mapView;

    int m_startBuildingId = -1;
    std::vector<int> m_destBuildingIds; // 👇 终点变成了一个数组
};

} // namespace controller