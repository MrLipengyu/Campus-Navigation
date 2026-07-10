#pragma once
#include <QVector>
#include <QPainter>
#include <QSize>

namespace graphics {

// 天气类型枚举
enum class WeatherType { Sunny, Rain, Snow };

// 单个粒子数据（雨滴或雪花）
struct Particle {
    float x, y;     // 当前位置（视口坐标，px）
    float vx, vy;   // 速度向量（px/帧）
    float alpha;    // 透明度 0.0~1.0
    float size;     // 雨滴长度 or 雪花半径（px）
    float phase;    // 雪花摇曳相位（弧度），让不同雪花左右飘的节奏不同
};

// 天气粒子系统
// 职责：管理粒子池的物理更新和 QPainter 绘制，不依赖 QGraphicsScene
class WeatherSystem {
public:
    WeatherSystem() = default;

    // 切换天气类型，会清空并重置粒子池
    void setWeather(WeatherType type);
    WeatherType currentWeather() const { return m_type; }

    // 每帧调用：推进粒子物理模拟，viewportSize 用于边界环绕
    void update(const QSize& viewportSize);

    // 在 paintEvent 中调用：将粒子绘制到视口 QPainter 上
    void render(QPainter& painter, const QSize& viewportSize) const;

private:
    // 重置单个粒子到随机初始状态
    // randomY=true 时 y 随机分布全屏（初始化散布用），false 时从顶部生成
    void resetParticle(Particle& p, const QSize& size, bool randomY);

    WeatherType   m_type = WeatherType::Sunny;
    QVector<Particle> m_particles;
    float         m_time = 0.0f; // 全局时间，用于雪花摇曳正弦计算

    static constexpr int RAIN_COUNT = 200; // 同屏雨滴数
    static constexpr int SNOW_COUNT = 120; // 同屏雪花数
};

} // namespace graphics
