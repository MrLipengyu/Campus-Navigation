#include "AStar.h"
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <cmath> // std::hypot

namespace core {

AStarPathfinder::AStarPathfinder(const Graph& graph) : m_graph(graph) {}

// 启发函数：计算 nodeId 到 goalId 的欧几里得直线距离
// 由于边权重 = 像素距离，此函数永远不会高估实际最短距离，满足 A* 可采纳性
double AStarPathfinder::heuristic(int nodeId, int goalId) const {
    const Node* n = m_graph.getNode(nodeId);
    const Node* g = m_graph.getNode(goalId);
    if (!n || !g) return 0.0;
    return std::hypot(static_cast<double>(n->x - g->x),
                      static_cast<double>(n->y - g->y));
}

std::vector<int> AStarPathfinder::findShortestPath(int startId, int endId) {
    std::vector<int> path;

    // 起点或终点不存在，直接返回空
    if (!m_graph.getNode(startId) || !m_graph.getNode(endId)) {
        return path;
    }

    // g(n)：从起点到节点 n 的实际已知最短距离
    std::unordered_map<int, double> gScore;
    // 记录最短路径树中的前驱节点，用于回溯
    std::unordered_map<int, int> previous;

    // 初始化所有节点的 g 值为无穷大
    for (const auto& [id, node] : m_graph.getAllNodes()) {
        gScore[id] = std::numeric_limits<double>::infinity();
    }
    gScore[startId] = 0.0;

    // 优先队列元素：{f(n), 节点ID}，f(n) = g(n) + h(n)
    // f 值越小越优先出队，引导搜索向终点方向进行
    using QueueEntry = std::pair<double, int>;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>> openSet;

    // 起点的 f 值 = g(start) + h(start) = 0 + h(start, end)
    openSet.push({heuristic(startId, endId), startId});

    while (!openSet.empty()) {
        auto [fCurrent, currentId] = openSet.top();
        openSet.pop();

        // 提前退出优化：到达终点，无需继续搜索
        if (currentId == endId) break;

        // 过期节点剪枝：如果弹出的 f 值对应的 g 值已经不是最优，跳过
        // 注意：这里用 fCurrent - h(current) 估算来过滤，但更准确的做法是记录 closedSet
        // 此处用更简单的"实际 g 值不一致"来过滤过期条目（与 Dijkstra 的过期过滤类似）
        double expectedG = fCurrent - heuristic(currentId, endId);
        if (expectedG > gScore[currentId] + 1e-9) continue;

        const Node* currentNode = m_graph.getNode(currentId);
        if (!currentNode) continue;

        // 遍历所有邻居，尝试松弛操作
        for (const auto& edge : currentNode->edges) {
            double tentativeG = gScore[currentId] + edge.distance;

            // 松弛操作：发现更短路径则更新
            if (tentativeG < gScore[edge.toNodeId]) {
                gScore[edge.toNodeId] = tentativeG;
                previous[edge.toNodeId] = currentId;

                // f(n) = g(n) + h(n)：A* 的核心，带方向感的启发式估算
                double fScore = tentativeG + heuristic(edge.toNodeId, endId);
                openSet.push({fScore, edge.toNodeId});
            }
        }
    }

    // 终点不可达
    if (gScore.find(endId) == gScore.end() ||
        gScore[endId] == std::numeric_limits<double>::infinity()) {
        return path;
    }

    // 安全回溯：使用 find() 避免 operator[] 插入默认值 0 导致死循环
    for (int at = endId; at != startId; ) {
        path.push_back(at);
        auto it = previous.find(at);
        if (it == previous.end()) {
            // 回溯链断裂，防御性返回空路径
            path.clear();
            return path;
        }
        at = it->second;
    }
    path.push_back(startId);

    // 倒推结果翻转为正向路径：起点 -> 终点
    std::reverse(path.begin(), path.end());

    return path;
}

} // namespace core
