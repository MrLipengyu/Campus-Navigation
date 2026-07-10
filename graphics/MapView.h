#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer> //定时器
#include <QSet>   //集合容器，用来装按下的按键
#include <QVector>

#include "CharacterItem.h" //引入角色头文件
#include "NpcItem.h"       //引入 NPC 头文件
#include "WeatherSystem.h" //引入天气粒子系统

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

    // 获取角色的指针（为了后续自动导航用）
    CharacterItem* getCharacter() const { return m_character; }

    // 让外界控制角色速度
    void setCharacterSpeed(qreal speed);

    // 启动和停止自动导航的接口
    void startAutoNavigation(const std::vector<int>& pathNodeIds);
    void stopAutoNavigation();

    // 向外提供切换昼夜的接口
    void setNightMode(bool isNight);

    // 添加 NPC 到地图
    void addNpc(NpcItem* npc);

    // 对话状态切换（true=冻结WASD）
    void setTalkingMode(bool isTalking);

    // ========== 天气特效控制接口 ==========
    void setWeather(WeatherType type);    // 手动切换天气
    void setRandomWeather(bool enabled);  // 开启/关闭随机天气事件
    WeatherType currentWeather() const;   // 查询当前天气

signals:
    void buildingClicked(int buildingId);

    // 当角色走到终点时，发射此信号通知外界
    void autoNavigationFinished();

    // 当玩家靠近 NPC 时，发射此信号
    void npcTriggered(NpcItem* npc);

    // 天气变化时通知 UI 同步 RadioButton（随机天气模式下自动触发）
    void weatherChanged(WeatherType type);

    // 昨夜自动切换时发射（通知 UI 更新状态标签）
    void dayNightChanged(bool isNight);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;

    // 不再在 KeyPress 里直接移动，而是记录按键状态
    void keyPressEvent(QKeyEvent *event) override;
    // 按键松开事件
    void keyReleaseEvent(QKeyEvent *event) override;
    // 天气粒子覆盖绘制：在场景之上叠加全屏粒子
    void paintEvent(QPaintEvent* event) override;

    // 绘制小地图（在 paintEvent 中调用）
    void drawMiniMap(QPainter& painter, const QSize& viewportSize);

private slots: // 必须是 slot，才能和 QTimer 配合
    // 🌟 游戏主循环：每 16ms 触发一次，处理平滑移动和相机跟随
    void gameLoop();
    // 随机天气定时器触发：按权重随机选取新天气
    void onRandomWeatherTick();
    // 每分钟读取系统本地时间，自动切换昼夜
    void syncDayNightWithSystemTime();

private:
    void setupBackground();
    void renderGraph();     // 现在只画路（线段）和纯节点（小圆点）
    void renderBuildings(); // 新增：专门画建筑（名字）

    // 夜间滤镜图元
    QGraphicsRectItem* m_nightOverlay = nullptr;

    // 小地图底图缓存
    QPixmap m_minimapPixmap;

private:
    QGraphicsScene* m_scene;
    const core::CampusMap& m_campusMap; // 核心数据源变成了 CampusMap

    // 用于保存当前高亮路径图元的指针，方便后续清除
    std::vector<QGraphicsItem*> m_pathItems;

    CharacterItem* m_character; // 🚶 我们的主角

    // 状态机数据
    QSet<int> m_pressedKeys; // 记录当前一直被按住的按键 (如 Qt::Key_W)
    QTimer* m_gameTimer;     // 驱动移动的时钟引擎

    // ================= 自动导航状态机数据 =================
    bool m_isAutoNavigating = false;           // 当前是否处于自动驾驶模式
    std::vector<QPointF> m_waypoints;          // 航点坐标列表
    size_t m_currentWaypointIndex = 0;         // 当前正在前往第几个航点

    // ================= NPC 系统 =================
    QVector<NpcItem*> m_npcList;  // 地图上所有 NPC 的指针列表
    bool m_isTalking = false;     // 对话进行中（true 时冻结 WASD）

    // ================= 天气特效系统 =================
    WeatherSystem m_weather;              // 粒子天气系统（值成员，无需堆分配）
    QTimer*       m_weatherTimer = nullptr;    // 随机天气单次定时器
    bool          m_randomWeatherEnabled = false; // 随机天气开关

    // ================= 昼夜自动同步系统 =================
    QTimer* m_dayNightSyncTimer = nullptr; // 每分钟读取系统时间，自动切换昼夜
    bool    m_isNight = false;             // 当前昼夜状态（防止重复刻新）
};

} // namespace graphics