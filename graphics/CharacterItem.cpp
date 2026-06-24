#include "CharacterItem.h"

namespace graphics {

CharacterItem::CharacterItem(QGraphicsItem* parent)
    : QGraphicsObject(parent) {
    // 设置 Z-Value 为 5.0，确保人物永远在地图、建筑标签和导航红线的最上层！
    setZValue(5.0);
}

QRectF CharacterItem::boundingRect() const {
    // 返回一个包围角色的矩形，Qt 靠这个矩形来知道什么时候需要刷新屏幕区域
    // 稍微给大一点点边界，防止重绘时边缘残留
    return QRectF(-m_radius - 2, -m_radius - 2, m_radius * 2 + 4, m_radius * 2 + 4);
}

void CharacterItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // 开启抗锯齿，让角色看起来圆润
    painter->setRenderHint(QPainter::Antialiasing);

    // 1. 画角色的身体（蓝色的圆）
    painter->setBrush(QColor(52, 152, 219)); // 经典的彼得潘蓝
    painter->setPen(QPen(Qt::white, 2));     // 白色描边
    painter->drawEllipse(QRectF(-m_radius, -m_radius, m_radius * 2, m_radius * 2));

    // 2. 画角色的“脸”或“朝向”（用一个小黄点表示他面朝前方）
    painter->setBrush(QColor(241, 196, 15)); // 黄色
    painter->setPen(Qt::NoPen);
    // 画在圆形的偏上方，表示默认朝上
    painter->drawEllipse(QRectF(-4, -m_radius + 2, 8, 8));
}

void CharacterItem::moveByOffset(qreal dx, qreal dy) {
    // moveBy 是 QGraphicsItem 自带的方法，用于相对当前位置移动
    moveBy(dx * m_speed, dy * m_speed);
}

// 👇 新增方法实现
void CharacterItem::setSpeed(qreal speed) {
    m_speed = speed;
}

} // namespace graphics