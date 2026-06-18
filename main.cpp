#include <QApplication>
#include <QDebug>
#include <iostream>
#include <windows.h>

#include "core/graph/Graph.h"
#include "core/pathfinding/Dijkstra.h"
#include "graphics/MapView.h"

// 辅助打印函数（纯 C++ 控制台输出，方便调试）
void printGraphStructure(const core::Graph& graph) {
    std::cout << "\n========== 天津理工大学校园图结构验证 ==========\n";

    const auto& allNodes = graph.getAllNodes();
    for (const auto& [id, node] : allNodes) {
        std::cout << "地点 [" << id << "]: " << node.name
                  << " (坐标: " << node.x << ", " << node.y << ")\n";

        if (node.edges.empty()) {
            std::cout << "  [孤立节点，暂无通路]\n";
        } else {
            // 使用 C++17 结构化绑定遍历边
            for (const auto& [toNodeId, distance] : node.edges) {
                // 去图中查找目标节点的名字
                const auto* targetNode = graph.getNode(toNodeId);
                std::string targetName = targetNode ? targetNode->name : "未知地点";

                std::cout << "  ---> 连接到: " << targetName
                          << " (ID: " << toNodeId << ", 距离: " << distance << "米)\n";
            }
        }
        std::cout << "-------------------------------------------\n";
    }
}

int main(int argc, char *argv[]) {

    SetConsoleOutputCP(CP_UTF8);

    QApplication app(argc, argv);

    qDebug() << "Campus Navigator Initialized.";
    qDebug() << "Running with Qt Version:" << qVersion();

    // 1. 实例化图
    core::Graph tuteGraph;

    // 2. 添加天津理工大学的几个核心节点 (ID, 名称, 简介, X坐标, Y坐标)
    tuteGraph.addNode(1, "正大门", "天津理工大学主校门", 100, 500);
    tuteGraph.addNode(2, "图书馆", "学校的地标建筑，藏书丰富", 300, 400);
    tuteGraph.addNode(3, "第一食堂", "靠近宿舍区的美食聚集地", 200, 200);
    tuteGraph.addNode(4, "明理楼", "主要的教学楼之一", 500, 450);

    // 3. 添加无向边 (起始ID, 目标ID, 实际距离)
    tuteGraph.addUndirectedEdge(1, 2, 150.5); // 大门 <-> 图书馆
    tuteGraph.addUndirectedEdge(2, 3, 200.0); // 图书馆 <-> 一食堂
    tuteGraph.addUndirectedEdge(2, 4, 100.0); // 图书馆 <-> 明理楼
    tuteGraph.addUndirectedEdge(3, 4, 350.2); // 一食堂 <-> 明理楼

    // 4. 执行结构打印验证
    printGraphStructure(tuteGraph);

    // 5. 验证只读保护：尝试通过 getNode 获取节点并读取
    const auto* lib = tuteGraph.getNode(2);
    if (lib) {
        std::cout << "\n单点查询成功！ID 2 是: " << lib->name << "\n";
        // lib->name = "非法修改"; // 如果取消注释这行，编译会报错，证明了我们的 Class+Struct 架构安全有效
    }

    std::cout << "===========================================\n\n";

    std::cout << "\n========== Dijkstra 最短路径算法测试 ==========\n";
    core::Pathfinder pathfinder(tuteGraph);

    // 测试：从 正大门(1) 到 明理楼(4)
    std::vector<int> shortestPath = pathfinder.findShortestPath(1, 4);

    std::cout << "从 正大门 到 明理楼 的最短路线规划：\n";
    if (shortestPath.empty()) {
        std::cout << "  无法到达！\n";
    } else {
        for (size_t i = 0; i < shortestPath.size(); ++i) {
            std::cout << tuteGraph.getNode(shortestPath[i])->name;
            if (i != shortestPath.size() - 1) {
                std::cout << " -> ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "===========================================\n";

    // 6. 启动 UI 视图
    std::cout << "\n启动地图可视化视图...\n";
    graphics::MapView mapView(tuteGraph);
    mapView.setWindowTitle("天津理工大学 - 校园导航系统 v1.0");
    mapView.resize(1024, 768); // 初始窗口大小
    mapView.show();

    // 正式进入 Qt 的事件循环（窗口不会一闪而过）
    return app.exec();
}