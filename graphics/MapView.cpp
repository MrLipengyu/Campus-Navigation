#include "MapView.h"

#include <QKeyEvent>
#include <QDebug>
#include <cmath> // 用于 std::hypot 计算向量长度

namespace graphics {

MapView::MapView(const core::CampusMap& campusMap, QWidget* parent)
    : QGraphicsView(parent), m_campusMap(campusMap) {

    // 1. 初始化场景
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    // 2. 优化渲染质量与交互体验
    setRenderHint(QPainter::Antialiasing); // 开启抗锯齿，线条更平滑
    setDragMode(QGraphicsView::ScrollHandDrag); // 允许鼠标拖拽平移地图
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏滚动条
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 3. 按层级顺序绘制内容
    setupBackground();
    renderGraph();     // 画纯路网
    renderBuildings(); // 画业务建筑

    // ================== 🌟 新增主角登场 ==================
    m_character = new CharacterItem();
    m_scene->addItem(m_character);

    // 我们把小人的初始位置放到“正大门”或者地图中心，这里暂时设为坐标 (800, 400)
    // 后续你可以根据你的 JSON 数据选一个大门的 ui_x 和 ui_y
    m_character->setPos(800, 400);

    // ================== 🌟 初始化游戏引擎 ==================
    m_gameTimer = new QTimer(this);
    // 将定时器的 timeout 信号连接到我们的 gameLoop 函数上
    connect(m_gameTimer, &QTimer::timeout, this, &MapView::gameLoop);
    // 开启定时器，16毫秒执行一次，即 1000ms / 16ms ≈ 60 FPS (60帧/秒)
    m_gameTimer->start(16);
}

void MapView::setupBackground() {
    QPixmap mapPixmap(":/campus_map.png");
    if (!mapPixmap.isNull()) {
        QGraphicsPixmapItem* bgItem = m_scene->addPixmap(mapPixmap);
        bgItem->setZValue(0.0); // 最底层
        m_scene->setSceneRect(mapPixmap.rect());
    } else {
        qWarning() << "警告：未找到地图背景图片，请检查 qrc 资源配置！";
        m_scene->setSceneRect(0, 0, 1920, 1080);
    }
}

void MapView::renderGraph() {
    const auto& graph = m_campusMap.getGraph();
    const auto& nodes = graph.getAllNodes();

    QPen edgePen(QColor(150, 150, 150, 120)); // 半透明浅灰色，避免抢风头
    edgePen.setWidth(3);

    QBrush nodeBrush(QColor(100, 100, 100, 180)); // 灰色节点
    QPen nodePen(Qt::NoPen);
    int nodeRadius = 4; // 节点画小一点，作为底层路网参考

    // 1. 画边 (Z-Value: 1.0)
    for (const auto& [id, node] : nodes) {
        for (const auto& edge : node.edges) {
            if (id < edge.toNodeId) { // 无向图防重复绘制
                const auto* toNode = graph.getNode(edge.toNodeId);
                if (toNode) {
                    QGraphicsLineItem* line = m_scene->addLine(
                        node.x, node.y, toNode->x, toNode->y, edgePen);
                    line->setZValue(1.0);
                }
            }
        }
    }

    // 2. 画节点 (Z-Value: 2.0)
    for (const auto& [id, node] : nodes) {
        QGraphicsEllipseItem* dot = m_scene->addEllipse(
            node.x - nodeRadius, node.y - nodeRadius,
            nodeRadius * 2, nodeRadius * 2,
            nodePen, nodeBrush);
        dot->setZValue(2.0);
    }
}

void MapView::renderBuildings() {
    const auto& buildings = m_campusMap.getAllBuildings();

    // 使用稍微小一点、粗体的无衬线字体，看起来更现代
    QFont font("Microsoft YaHei", 9, QFont::Bold);

    for (const auto& [id, b] : buildings) {
        // 1. 生成文字图元
        QGraphicsTextItem* textItem = m_scene->addText(QString::fromStdString(b.name), font);
        textItem->setDefaultTextColor(Qt::black); // 纯黑字

        // 2. 计算完美的背景框大小（包含 Padding 内边距）
        qreal paddingX = 6.0;
        qreal paddingY = 2.0;
        QRectF textRect = textItem->boundingRect();
        qreal bgWidth = textRect.width() + paddingX * 2;
        qreal bgHeight = textRect.height() + paddingY * 2;

        // 计算中心点坐标，使标签的中心对准 JSON 里的 ui_x 和 ui_y
        qreal x = b.ui_x - bgWidth / 2.0;
        qreal y = b.ui_y - bgHeight / 2.0;

        // 3. 画一个漂亮的圆角矩形背景
        QPainterPath path;
        path.addRoundedRect(QRectF(x, y, bgWidth, bgHeight), 4.0, 4.0); // 4.0是圆角半径

        QGraphicsPathItem* bgItem = m_scene->addPath(path,
                                                     QPen(QColor(150, 150, 150, 180), 1),     // 浅灰色边框
                                                     QBrush(QColor(255, 255, 255, 220)));     // 半透明白色背景

        // 4. 设置层级
        bgItem->setZValue(3.0);

        // 文字需要稍微偏移一点来居中对齐到背景框里
        textItem->setPos(x + paddingX, y + paddingY);
        textItem->setZValue(3.1); // 文字在背景框之上

        // 🌟【架构铺垫】：利用 setData 把建筑的真实 ID 塞进这两个图元的肚子里！
        // 这样以后用户无论点到背景还是文字，我们都能直接提取出 ID，而不用去算坐标！
        bgItem->setData(0, b.id);
        textItem->setData(0, b.id);
    }
}

void MapView::drawPath(const std::vector<int>& pathNodeIds) {
    clearPath(); // 画新路径前先清空老路径

    if (pathNodeIds.size() < 2) return;

    const auto& graph = m_campusMap.getGraph();

    // 设置高亮画笔：粗壮、红色、圆滑线帽和拐角
    QPen pathPen(QColor(255, 50, 50, 220));
    pathPen.setWidth(6);
    pathPen.setCapStyle(Qt::RoundCap);
    pathPen.setJoinStyle(Qt::RoundJoin);

    for (size_t i = 0; i < pathNodeIds.size() - 1; ++i) {
        const auto* n1 = graph.getNode(pathNodeIds[i]);
        const auto* n2 = graph.getNode(pathNodeIds[i+1]);

        if (n1 && n2) {
            QGraphicsLineItem* line = m_scene->addLine(
                n1->x, n1->y, n2->x, n2->y, pathPen);
            line->setZValue(4.0); // 最高层级！不会被建筑或底图遮挡
            m_pathItems.push_back(line); // 存入容器以便后续销毁
        }
    }
}

void MapView::clearPath() {
    // 内存管理最佳实践：从场景移除并 delete
    for (QGraphicsItem* item : m_pathItems) {
        m_scene->removeItem(item);
        delete item;
    }
    m_pathItems.clear();
}

// 👇 新增方法实现
void MapView::setCharacterSpeed(qreal speed) {
    if (m_character) {
        m_character->setSpeed(speed);
    }
}

void MapView::wheelEvent(QWheelEvent* event) {
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}

// 👇 1. 按下按键：把按键记在“小本本”上
void MapView::keyPressEvent(QKeyEvent *event) {
    // 忽略操作系统的“自动重复”长按机制，我们只关心真实的物理按下
    if (!event->isAutoRepeat()) {
        m_pressedKeys.insert(event->key());
    }
}

// 👇 2. 松开按键：从“小本本”上擦除
void MapView::keyReleaseEvent(QKeyEvent *event) {
    if (!event->isAutoRepeat()) {
        m_pressedKeys.remove(event->key());
    }
}

// 👇 3. 🌟 核心引擎：每 16ms 自动调用一次！
void MapView::gameLoop() {
    // 如果没有按键被按下，直接返回，什么都不做
    if (m_pressedKeys.isEmpty()) return;

    qreal dx = 0.0;
    qreal dy = 0.0;

    // 检查集合里同时包含哪些按键，实现八向移动
    if (m_pressedKeys.contains(Qt::Key_W)) dy -= 1.0;
    if (m_pressedKeys.contains(Qt::Key_S)) dy += 1.0;
    if (m_pressedKeys.contains(Qt::Key_A)) dx -= 1.0;
    if (m_pressedKeys.contains(Qt::Key_D)) dx += 1.0;

    // 如果 x 和 y 都没有变化，说明可能同时按下了 W 和 S 抵消了
    if (dx != 0.0 || dy != 0.0) {

        // 【高级技巧：向量归一化】
        // 如果不做处理，斜着走（比如右下 dx=1, dy=1）的速度会是直走的 1.414 倍。
        // 用勾股定理计算出向量长度，然后将 dx 和 dy 除以这个长度，保证八个方向速度绝对一致！
        double length = std::hypot(dx, dy);
        dx /= length;
        dy /= length;

        // 命令角色移动（因为是 60帧/秒，所以如果你觉得现在走得太快/太慢，
        // 请去 CharacterItem.h 里把 m_speed 调小，比如调成 3.0 或 5.0）
        m_character->moveByOffset(dx, dy);

        // 🌟【关键修复】：强行让 MapView 的镜头中心对准角色的当前位置！
        centerOn(m_character);
    }
}

void MapView::mousePressEvent(QMouseEvent* event) {
    // 1. 获取鼠标在视图中的点击位置，并找到该位置最顶层的图元 (Item)
    QGraphicsItem* clickedItem = itemAt(event->pos());

    // 2. 判断是否点到了东西
    if (clickedItem) {
        // 3. 尝试读取我们在 renderBuildings 中用 setData 埋进去的建筑 ID
        // data(0) 对应我们在 setData(0, b.id) 中设定的键值
        QVariant itemData = clickedItem->data(0);

        // 4. 如果读取到的数据有效（说明点到的是建筑标签或背景框）
        if (itemData.isValid()) {
            int buildingId = itemData.toInt(); // 转换为 int 型的建筑 ID
            qDebug() << "[UI 层] 捕获到鼠标点击，建筑 ID:" << buildingId;

            // 5. 🌟 核心操作：发射信号，将 ID 广播出去！Controller 会监听这个信号
            emit buildingClicked(buildingId);
        }
    }

    // 6. 别忘了调用父类的默认处理逻辑，保证地图依然可以被鼠标拖拽
    QGraphicsView::mousePressEvent(event);
}

} // namespace graphics