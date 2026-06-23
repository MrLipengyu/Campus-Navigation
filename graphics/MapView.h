#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include "../core/map/CampusMap.h" // 引入 CampusMap

namespace graphics {

class MapView : public QGraphicsView {
    Q_OBJECT

public:
    explicit MapView(const core::CampusMap& campusMap, QWidget* parent = nullptr);

    // 新增：绘制高亮导航路径的方法
    void drawPath(const std::vector<int>& pathNodeIds);
    // 新增：清除高亮路径
    void clearPath();

signals:
    void buildingClicked(int buildingId);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setupBackground();
    void renderGraph();     // 现在只画路（线段）和纯节点（小圆点）
    void renderBuildings(); // 新增：专门画建筑（名字）

private:
    QGraphicsScene* m_scene;
    const core::CampusMap& m_campusMap; // 核心数据源变成了 CampusMap

    // 用于保存当前高亮路径图元的指针，方便后续清除
    std::vector<QGraphicsItem*> m_pathItems;
};

} // namespace graphics