#include "CharacterItem.h"
#include <algorithm> // 👇 新增：必须包含这个标准库，才能使用 C++17 的 std::clamp
#include <QGraphicsScene> // 👇 新增：为了获取 scene()

namespace graphics {

CharacterItem::CharacterItem(QGraphicsItem* parent)
    : QGraphicsObject(parent),
      m_currentDir(Down), m_currentFrame(0),
      m_isMoving(false), m_animationTick(0)
{
    setZValue(5.0);

    // 1. 加载精灵图（请确保文件路径与你的 qrc 中一致！）
    if (!m_spriteSheet.load(":/character.png")) {
        qWarning("警告：未找到角色精灵图，请检查资源文件！");
    }

    // 2. 假设你的精灵图是标准的 4行(方向) x 4列(动作帧)
    // 请根据你找的实际图片修改 m_maxFrames 的值（3 或 4）
    m_maxFrames = 8;

    if (!m_spriteSheet.isNull()) {
        // 宽度除以 8 (列)，高度除以 4 (行)
        m_frameWidth = m_spriteSheet.width() / m_maxFrames;
        m_frameHeight = m_spriteSheet.height() / 4;
    } else {
        m_frameWidth = 32;
        m_frameHeight = 48;
    }
}

QRectF CharacterItem::boundingRect() const {
    // 绘图区域就是单个动作帧的大小。
    // 为了让角色的脚底对准坐标点，我们把原点 (0,0) 设在角色脚底中心。
    return QRectF(-m_frameWidth / 2.0, -m_frameHeight, m_frameWidth, m_frameHeight);
}

void CharacterItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    if (m_spriteSheet.isNull()) {
        // 如果图没加载成功，画个红框提示你
        painter->setBrush(Qt::red);
        painter->drawRect(boundingRect());
        return;
    }

    // 🌟 核心魔法：只裁剪精灵图的一小块进行绘制
    // 目标区域：画在我们定义的 boundingRect 上
    QRectF targetRect = boundingRect();

    // 源图区域：计算当前帧在原大图上的坐标
    qreal sourceX = m_currentFrame * m_frameWidth;
    qreal sourceY = m_currentDir * m_frameHeight;
    QRectF sourceRect(sourceX, sourceY, m_frameWidth, m_frameHeight);

    // 将原图的 sourceRect 部分，画到场景的 targetRect 上
    painter->drawPixmap(targetRect, m_spriteSheet, sourceRect);
}

void CharacterItem::moveByOffset(qreal dx, qreal dy) {
    // 1. 根据当前速度和方向，计算出角色【期望】的新坐标
    qreal newX = x() + dx * m_speed;
    qreal newY = y() + dy * m_speed;

    // 2. 边界检测（空气墙逻辑）
    // 如果角色已经被添加到了地图场景中，scene() 将不会为空
    if (scene()) {
        // 获取当前地图场景的真实物理边界（也就是你的 1774x887）
        QRectF mapBounds = scene()->sceneRect();

        // 计算内边距，防止小人“半个身子”出界
        // 因为我们的原点 (0,0) 设置在小人脚底正中心，所以：
        qreal paddingX = m_frameWidth / 2.0;  // 左右各留半个身位
        qreal paddingY = m_frameHeight;       // 顶部留一整个身高（防止头出界）
        qreal bottomPadding = 5.0;            // 底部留一点点脚底边距

        // 🌟 核心魔法：使用 C++17 的 std::clamp 钳制坐标
        // std::clamp(value, min, max) 会保证 value 永远在 min 和 max 之间
        newX = std::clamp(newX, mapBounds.left() + paddingX, mapBounds.right() - paddingX);
        newY = std::clamp(newY, mapBounds.top() + paddingY, mapBounds.bottom() - bottomPadding);
    }

    // 3. 应用最终的合法坐标（不再使用相对的 moveBy，而是绝对的 setPos）
    setPos(newX, newY);
}

void CharacterItem::setSpeed(qreal speed) {
    m_speed = speed;
}

// 被 MapView 的 60帧循环调用
void CharacterItem::updateAnimationState(qreal dx, qreal dy) {
    // 1. 判定是否在移动
    if (dx == 0 && dy == 0) {
        m_isMoving = false;
        m_currentFrame = 0; // 停下来时，恢复站立帧 (通常是第0帧或第1帧)
        update();           // 通知 Qt 重绘
        return;
    }

    m_isMoving = true;

    // 2. 根据移动向量决定朝向
    // 优先判断 Y 轴（上下），如果斜着走，视觉上通常呈现上下朝向比较自然
    // 优先判断 Y 轴（上下）
    if (dy > 0.1) {
        m_currentDir = Down;
    } else if (dy < -0.1) {
        m_currentDir = Up;
    } else if (dx > 0.1) {
        m_currentDir = Right;
    } else if (dx < -0.1) {
        m_currentDir = Left;
    }

    // 3. 动画切帧降速器 (控制小人的“倒腾腿”频率)
    m_animationTick++;
    int frameSpeed = (m_speed > 5.0) ? 3 : 6; // 跑得快，切帧就快(5tick)；走得慢，切帧就慢(10tick)

    if (m_animationTick >= frameSpeed) {
        m_animationTick = 0;
        m_currentFrame++; // 切换到下一帧
        if (m_currentFrame >= m_maxFrames) {
            m_currentFrame = 0; // 循环播放
        }
    }

    // 更新画面
    update();
}

} // namespace graphics