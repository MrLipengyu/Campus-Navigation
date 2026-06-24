#pragma once
#include <QGraphicsObject>
#include <QPainter>

namespace graphics {

// 继承自 QGraphicsObject，为后续的自动导航动画做准备
class CharacterItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit CharacterItem(QGraphicsItem* parent = nullptr);
    ~CharacterItem() = default;

    // 必须重写的虚函数：定义角色的碰撞/刷新边界
    QRectF boundingRect() const override;

    // 必须重写的虚函数：决定角色长什么样
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // 手动控制角色移动的接口
    void moveByOffset(qreal dx, qreal dy);

    // 👇 新增：设置角色速度的接口
    void setSpeed(qreal speed);

private:
    qreal m_radius = 12.0; // 角色的半径大小
    qreal m_speed = 2.0;  // 每次按下 WASD 的移动步长
};

} // namespace graphics