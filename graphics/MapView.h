#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include "../core/graph/Graph.h"

namespace graphics {

class MapView : public QGraphicsView {
    Q_OBJECT // Qt的宏，用于支持信号与槽

public:
    // 构造函数：传入底层的Graph常量引用
    explicit MapView(const core::Graph& graph, QWidget* parent = nullptr);
    ~MapView() = default;

protected:
    // 重写滚轮事件，实现地图的缩放
    void wheelEvent(QWheelEvent* event) override;

private:
    // 初始化地图背景
    void setupBackground();
    // 渲染图结构的节点和边
    void renderGraph();

private:
    QGraphicsScene* m_scene;
    const core::Graph& m_graph;
};

} // namespace graphics