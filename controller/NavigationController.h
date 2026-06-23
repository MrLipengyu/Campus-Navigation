#pragma once
#include <QObject>
#include "../core/map/CampusMap.h"
#include "../graphics/MapView.h"

namespace controller {

class NavigationController : public QObject {
    Q_OBJECT

public:
    NavigationController(const core::CampusMap& campusMap, graphics::MapView& mapView, QObject* parent = nullptr);

public slots:
    // 明确的显式指令槽函数
    void setStartNode(int buildingId);
    void setEndNode(int buildingId);

private:
    // 检查状态并尝试寻路
    void tryPlanRoute();
    void planAndDrawRoute();

private:
    const core::CampusMap& m_campusMap;
    graphics::MapView& m_mapView;

    int m_startBuildingId = -1;
    int m_endBuildingId = -1;
};

} // namespace controller