#include "NpcItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

namespace graphics {

NpcItem::NpcItem(const NpcData& data, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_data(data)
{
    // 加载 NPC 图片资源
    if (!m_pixmap.load(":/npc.png")) {
        qWarning("[NpcItem] 警告：未找到 npc.png，请检查资源文件！");
    } else {
        m_pixWidth  = m_pixmap.width();
        m_pixHeight = m_pixmap.height();
    }

    // 设置场景坐标（脚底对准坐标点）
    setPos(m_data.position);

    // NPC 可以被点击
    setAcceptedMouseButtons(Qt::LeftButton);

    // NPC 层级：在建筑标签(3.x)之上，在玩家(5.0)之下
    setZValue(4.0);

    qDebug() << "[NpcItem] NPC 已创建：" << m_data.name
             << "位置：" << m_data.position
             << "触发半径：" << m_data.triggerRadius;
}

QRectF NpcItem::boundingRect() const {
    // 以脚底中心为原点，图片向上展开
    return QRectF(-m_pixWidth / 2.0, -m_pixHeight, m_pixWidth, m_pixHeight);
}

void NpcItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    if (m_pixmap.isNull()) {
        // 图片加载失败：画一个醒目的紫色矩形作为占位
        painter->setBrush(QColor(160, 32, 240, 200));
        painter->setPen(Qt::NoPen);
        painter->drawRect(boundingRect());
        return;
    }

    // 将图片绘制在 boundingRect 区域内（脚底对齐）
    painter->drawPixmap(boundingRect().toRect(), m_pixmap);

    // 在 NPC 头顶绘制名字标签
    QFont font("Microsoft YaHei", 6); // 缩小字体：6pt 更小巧
    painter->setFont(font);

    QString label = m_data.displayName;
    QFontMetrics fm(font);
    int textW = fm.horizontalAdvance(label);
    int textH = fm.height();

    // 标签背景框（头顶上方 4px），内边距缩减为 2px
    int bgX = -textW / 2 - 2;
    int bgY = -m_pixHeight - textH - 6;
    QRect bgRect(bgX, bgY, textW + 4, textH + 2);

    painter->setBrush(QColor(30, 30, 50, 200));
    painter->setPen(QPen(QColor(120, 80, 255), 1));
    painter->drawRoundedRect(bgRect, 2, 2);

    // 绘制文字（白色）
    painter->setPen(Qt::white);
    painter->drawText(bgRect, Qt::AlignCenter, label);

    // 调试：可以画出触发范围圆（正式版去掉注释启用/禁用）
    // painter->setPen(QPen(QColor(255, 200, 0, 80), 1, Qt::DashLine));
    // painter->setBrush(Qt::NoBrush);
    // qreal r = m_data.triggerRadius;
    // painter->drawEllipse(QPointF(0, -m_pixHeight / 2.0), r, r);
}

void NpcItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    Q_UNUSED(event)
    qDebug() << "[NpcItem] 被点击：" << m_data.name;
    emit clicked(this);
}

void NpcItem::tickCooldown() {
    if (m_cooldownTicks > 0) {
        m_cooldownTicks--;
        if (m_cooldownTicks == 0) {
            m_triggered = false; // 冷却结束，可以再次触发
        }
    }
}

void NpcItem::startCooldown(int ticks) {
    m_triggered     = true;
    m_cooldownTicks = ticks;
}

} // namespace graphics
