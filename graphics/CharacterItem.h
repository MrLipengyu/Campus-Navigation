#pragma once
#include <QGraphicsObject>
#include <QPainter>
#include <QPixmap> // 新增：用于加载和处理图片

namespace graphics {

// 继承自 QGraphicsObject，为后续的自动导航动画做准备
class CharacterItem : public QGraphicsObject {
    Q_OBJECT

public:
    // 定义 4 个朝向（对应精灵图的 4 行，顺序需根据你的实际图片调整，通常RPG是下左右上）
    enum Direction {
        Down = 0,
        Up = 1,
        Left = 2,
        Right = 3
    };

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

    // 👇 新增：接收从 MapView 传来的移动状态，用于更新朝向和动画
    void updateAnimationState(qreal dx, qreal dy);

private:
    qreal m_speed = 2.0;  // 每次按下 WASD 的移动步长

    // ================= 动画相关属性 =================
    QPixmap m_spriteSheet;      // 精灵大图
    int m_frameWidth;           // 单个动作帧的宽度
    int m_frameHeight;          // 单个动作帧的高度

    Direction m_currentDir;     // 当前朝向
    int m_currentFrame;         // 当前播放到第几帧 (列)
    int m_maxFrames;            // 总帧数 (列数，一般是 3 或 4)

    bool m_isMoving;            // 角色当前是否在移动
    int m_animationTick;        // 动画降速计数器 (60FPS太快，需要放慢切帧速度)
};

} // namespace graphics