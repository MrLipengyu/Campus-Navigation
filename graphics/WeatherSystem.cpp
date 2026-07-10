#include "WeatherSystem.h"
#include <cmath>
#include <QRandomGenerator>

namespace graphics {

// ==================== 天气切换 ====================

void WeatherSystem::setWeather(WeatherType type) {
    m_type = type;
    m_particles.clear(); // 清空粒子池，让 update() 重新生成
    m_time = 0.0f;
}

// ==================== 粒子重置 ====================

void WeatherSystem::resetParticle(Particle& p, const QSize& size, bool randomY) {
    auto* rng = QRandomGenerator::global();

    // x 随机分布在视口宽度内（+40px 冗余避免右侧出现空白）
    p.x = static_cast<float>(rng->bounded(size.width() + 40));
    // randomY=true：初始化时让粒子散布全屏，避免开始时全部从顶部涌入
    // randomY=false：循环时从顶部上方重新生成，形成连续效果
    p.y = randomY
        ? static_cast<float>(rng->bounded(size.height()))
        : static_cast<float>(-(rng->bounded(40) + 5));

    // 摇曳相位：0 ~ 2π，让雪花个体差异化
    p.phase = static_cast<float>(rng->bounded(6284)) / 1000.0f;

    if (m_type == WeatherType::Rain) {
        // 雨滴：明显斜向右下，速度较快
        p.vx    = 1.2f + static_cast<float>(rng->bounded(100)) / 100.0f; // 1.2~2.2
        p.vy    = 8.0f + static_cast<float>(rng->bounded(400)) / 100.0f; // 8~12
        p.size  = 8.0f + static_cast<float>(rng->bounded(70))  / 10.0f;  // 8~15 px 线长
        p.alpha = static_cast<float>(100 + rng->bounded(60)) / 255.0f;   // 半透明蓝白
    } else {
        // 雪花：轻柔飘落，速度缓慢
        p.vx    = 0.0f;
        p.vy    = 0.8f + static_cast<float>(rng->bounded(220)) / 100.0f; // 0.8~3.0
        p.size  = 2.0f + static_cast<float>(rng->bounded(20))  / 10.0f;  // 2~4 px 半径
        p.alpha = static_cast<float>(180 + rng->bounded(50)) / 255.0f;   // 较不透明白色
    }
}

// ==================== 物理更新（每帧调用）====================

void WeatherSystem::update(const QSize& viewportSize) {
    if (m_type == WeatherType::Sunny) {
        m_particles.clear();
        return;
    }

    m_time += 0.016f; // 假设 60 FPS，约 16ms 每帧

    // 按需补充粒子（首次或切换天气后）
    int targetCount = (m_type == WeatherType::Rain) ? RAIN_COUNT : SNOW_COUNT;
    // 初始化阶段让粒子散布全屏（m_time 接近 0），后续从顶部生成
    bool initialScatter = (m_time < 0.05f);
    while (m_particles.size() < targetCount) {
        Particle p;
        resetParticle(p, viewportSize, initialScatter);
        m_particles.append(p);
    }

    // 推进每个粒子的位置
    for (auto& p : m_particles) {
        if (m_type == WeatherType::Rain) {
            // 雨滴：线性匀速斜落
            p.x += p.vx;
            p.y += p.vy;
        } else {
            // 雪花：正弦横向摇曳 + 垂直匀速飘落
            p.x += std::sin(m_time * 1.5f + p.phase) * 0.4f;
            p.y += p.vy;
        }

        // 粒子越界则重置到顶部（+40px 冗余处理斜向雨出界）
        if (p.y > viewportSize.height() + 20.0f ||
            p.x > viewportSize.width()  + 40.0f) {
            resetParticle(p, viewportSize, false);
        }
    }
}

// ==================== 渲染（paintEvent 中调用）====================

void WeatherSystem::render(QPainter& painter, const QSize& /*viewportSize*/) const {
    if (m_type == WeatherType::Sunny || m_particles.isEmpty()) return;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_type == WeatherType::Rain) {
        // 雨滴：绘制沿运动方向的短线段
        for (const auto& p : m_particles) {
            QColor color(180, 210, 255, static_cast<int>(p.alpha * 255));
            painter.setPen(QPen(color, 1.2));

            // 计算速度方向的单位向量，线段从当前位置向后延伸
            float mag = std::hypot(p.vx, p.vy);
            float nx = p.vx / mag;
            float ny = p.vy / mag;
            painter.drawLine(
                QPointF(p.x, p.y),
                QPointF(p.x - nx * p.size, p.y - ny * p.size)
            );
        }
    } else {
        // 雪花：绘制半透明白色实心圆点
        painter.setPen(Qt::NoPen);
        for (const auto& p : m_particles) {
            QColor color(240, 248, 255, static_cast<int>(p.alpha * 255));
            painter.setBrush(color);
            painter.drawEllipse(QPointF(p.x, p.y), p.size, p.size);
        }
    }

    painter.restore();
}

} // namespace graphics
