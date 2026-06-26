#pragma once
#include <QGraphicsObject>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QVector>

namespace graphics {

// NPC 的数据包（名字、对话内容、位置、触发半径）
struct NpcData {
    QString  name;            // NPC 识别名 (如 "mad_professor")
    QString  displayName;     // 对话框上显示的名字
    QPointF  position;        // 场景坐标
    qreal    triggerRadius;   // 靠近触发的距离（单位：像素）
};

// NPC 图元 —— 负责在地图上"站着"并被触发
class NpcItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit NpcItem(const NpcData& data, QGraphicsItem* parent = nullptr);

    // 必须重写：碰撞/刷新边界
    QRectF boundingRect() const override;

    // 必须重写：绘制 NPC
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // 获取 NPC 数据（供 MapView 判断触发）
    const NpcData& data() const { return m_data; }

    // 触发状态管理
    bool hasTriggered() const   { return m_triggered; }
    void setTriggered(bool val) { m_triggered = val; }

    // 冷却计时器（每帧 -1，归零后可再次触发）
    void tickCooldown();
    bool isInCooldown() const { return m_cooldownTicks > 0; }
    void startCooldown(int ticks = 180); // 默认 3 秒 (180帧@60FPS)

signals:
    // 被点击时发射
    void clicked(NpcItem* npc);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    NpcData  m_data;
    QPixmap  m_pixmap;

    int  m_pixWidth  = 48;
    int  m_pixHeight = 64;

    bool m_triggered     = false; // 当前对话是否已触发
    int  m_cooldownTicks = 0;     // 冷却倒计时（帧数）
};

} // namespace graphics
