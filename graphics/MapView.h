#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer> // 👇 新增：定时器
#include <QSet>   // 👇 新增：集合容器，用来装按下的按键

#include "CharacterItem.h" // 👇 引入角色头文件

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

    // 👇 新增：获取角色的指针（为了后续自动导航用）
    CharacterItem* getCharacter() const { return m_character; }

    // 👇 新增：让外界控制角色速度
    void setCharacterSpeed(qreal speed);

    // 👇 新增：启动和停止自动导航的接口
    void startAutoNavigation(const std::vector<int>& pathNodeIds);
    void stopAutoNavigation();

signals:
    void buildingClicked(int buildingId);

    // 👇 新增：当角色走到终点时，发射此信号通知外界
    void autoNavigationFinished();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;

    // 👇 修改：不再在 KeyPress 里直接移动，而是记录按键状态
    void keyPressEvent(QKeyEvent *event) override;
    // 👇 新增：按键松开事件
    void keyReleaseEvent(QKeyEvent *event) override;

private slots: // 👇 新增：必须是 slot，才能和 QTimer 配合
    // 🌟 游戏主循环：每 16ms 触发一次，处理平滑移动和相机跟随
    void gameLoop();

private:
    void setupBackground();
    void renderGraph();     // 现在只画路（线段）和纯节点（小圆点）
    void renderBuildings(); // 新增：专门画建筑（名字）

private:
    QGraphicsScene* m_scene;
    const core::CampusMap& m_campusMap; // 核心数据源变成了 CampusMap

    // 用于保存当前高亮路径图元的指针，方便后续清除
    std::vector<QGraphicsItem*> m_pathItems;

    CharacterItem* m_character; // 🚶 我们的主角

    // 👇 新增：状态机数据
    QSet<int> m_pressedKeys; // 记录当前一直被按住的按键 (如 Qt::Key_W)
    QTimer* m_gameTimer;     // 驱动移动的时钟引擎

    // ================= 自动导航状态机数据 =================
    bool m_isAutoNavigating = false;           // 当前是否处于自动驾驶模式
    std::vector<QPointF> m_waypoints;          // 航点坐标列表
    size_t m_currentWaypointIndex = 0;         // 当前正在前往第几个航点
};

} // namespace graphics