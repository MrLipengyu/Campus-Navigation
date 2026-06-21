#include "MapView.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QWheelEvent>
#include <QPen>
#include <QBrush>

namespace graphics {

MapView::MapView(const core::Graph& graph, QWidget* parent)
    : QGraphicsView(parent), m_graph(graph) {
    
    // 1. 初始化场景
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    // 2. 优化渲染质量与交互体验
    setRenderHint(QPainter::Antialiasing); // 开启抗锯齿
    setDragMode(QGraphicsView::ScrollHandDrag); // 允许鼠标拖拽平移地图
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏滚动条
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 3. 绘制内容
    setupBackground();
    renderGraph();
}

void MapView::setupBackground() {
    // 从资源文件中加载你的地图图片
    QPixmap mapPixmap(":/campus_map.png");
    
    if (!mapPixmap.isNull()) {
        m_scene->addPixmap(mapPixmap);
        // 设定场景大小与图片一致
        m_scene->setSceneRect(mapPixmap.rect()); 
    } else {
        // 如果图片加载失败，给个默认大小，方便调试
        m_scene->setSceneRect(0, 0, 1000, 800);
    }
}

void MapView::renderGraph() {
    const auto& nodes = m_graph.getAllNodes();

    // 准备画笔和画刷
    QPen edgePen(QColor(150, 150, 150, 180)); // 半透明灰色的道路
    edgePen.setWidth(4);
    
    QBrush nodeBrush(QColor(65, 105, 225));   // 皇家蓝的节点
    QPen nodePen(Qt::white);                  // 节点白边
    nodePen.setWidth(2);

    // 1. 先画所有的边 (压在节点下面)
    for (const auto& [id, node] : nodes) {
        for (const auto& edge : node.edges) {
            // 避免无向图重复画线：只画 id < toNodeId 的线
            if (id < edge.toNodeId) {
                const auto* toNode = m_graph.getNode(edge.toNodeId);
                if (toNode) {
                    m_scene->addLine(node.x, node.y, toNode->x, toNode->y, edgePen);
                }
            }
        }
    }

    // 2. 再画所有的节点 (盖在边上面)
    int radius = 10; // 节点半径
    for (const auto& [id, node] : nodes) {
        //判断是否为建筑节点（空节点不画）
        if (node.name.empty()) {
            continue;
        }
        // 绘制圆形 (注意：Qt绘图的坐标是以左上角为起点的，所以需要偏移半径让中心对齐x,y)
        m_scene->addEllipse(node.x - radius, node.y - radius, 
                            radius * 2, radius * 2, 
                            nodePen, nodeBrush);

        // 绘制文字标签
        QGraphicsTextItem* textItem = m_scene->addText(QString::fromStdString(node.name));
        textItem->setDefaultTextColor(Qt::black);
        // 将文字放在节点正下方
        textItem->setPos(node.x - textItem->boundingRect().width() / 2, node.y + radius);
    }
}

void MapView::wheelEvent(QWheelEvent* event) {
    // 实现鼠标滚轮缩放逻辑
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor); // 放大
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor); // 缩小
    }
}
void MapView::mousePressEvent(QMouseEvent* event) {
    // 将鼠标点击的窗口坐标，转换为地图图片的真实像素坐标
    QPointF scenePos = mapToScene(event->pos());

    // 1. 自动生成 JSON 输出到控制台
    qDebug().noquote() << "{ \"id\": " << m_debugId << ", \"name\": \"\", \"x\": "
                       << (int)scenePos.x() << ", \"y\": " << (int)scenePos.y() << " },";

    // 2. 🌟 神奇魔法：在地图上留下一个临时的红色记号和 ID！
    int radius = 5;
    m_scene->addEllipse(scenePos.x() - radius, scenePos.y() - radius, radius*2, radius*2,
                        QPen(Qt::red), QBrush(Qt::red));

    QGraphicsTextItem* textItem = m_scene->addText(QString::number(m_debugId));
    textItem->setDefaultTextColor(Qt::blue); // 用蓝色大字显示 ID
    QFont font = textItem->font();
    font.setPointSize(10);
    font.setBold(true);
    textItem->setFont(font);
    textItem->setPos(scenePos.x() + 5, scenePos.y() - 15);

    // 3. ID 自动 +1，为下一次点击做准备
    m_debugId++;

    // 保持原有的拖拽功能正常工作
    QGraphicsView::mousePressEvent(event);
}

} // namespace graphics