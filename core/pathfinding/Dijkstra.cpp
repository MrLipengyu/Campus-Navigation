#include "Dijkstra.h"
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>

namespace core {

Pathfinder::Pathfinder(const Graph& graph) : m_graph(graph) {}

std::vector<int> Pathfinder::findShortestPath(int startId, int endId) {
    // 结果路径
    std::vector<int> path;

    // 如果起点或终点在图中不存在，直接返回空
    if (!m_graph.getNode(startId) || !m_graph.getNode(endId)) {
        return path;
    }

    // 记录从起点到每个节点的最短已知距离
    std::unordered_map<int, double> distances;
    // 记录最短路径树中的前驱节点，用于回溯生成完整路径
    std::unordered_map<int, int> previous;

    // 初始化所有节点的距离为无穷大
    for (const auto& [id, node] : m_graph.getAllNodes()) {
        distances[id] = std::numeric_limits<double>::infinity();
    }
    distances[startId] = 0.0;

    // 定义优先队列的元素类型：{距离, 节点ID}
    using NodeRecord = std::pair<double, int>;
    // 使用最小堆，距离越小越优先出队
    std::priority_queue<NodeRecord, std::vector<NodeRecord>, std::greater<NodeRecord>> pq;

    pq.push({0.0, startId});

    while (!pq.empty()) {
        auto [currentDist, currentId] = pq.top();
        pq.pop();

        // 优化：如果弹出的节点就是终点，算法可以直接提前结束
        if (currentId == endId) break;

        // 如果弹出的距离已经大于已知的最短距离，说明是过期的数据，直接丢弃
        if (currentDist > distances[currentId]) continue;

        const Node* currentNode = m_graph.getNode(currentId);
        if (!currentNode) continue;

        // 遍历当前节点的所有邻居
        for (const auto& edge : currentNode->edges) {
            double newDist = currentDist + edge.distance;

            // 松弛操作 (Relaxation)：如果找到了更短的路径，则更新
            if (newDist < distances[edge.toNodeId]) {
                distances[edge.toNodeId] = newDist;
                previous[edge.toNodeId] = currentId;
                // 将更新后的记录压入优先队列
                pq.push({newDist, edge.toNodeId});
            }
        }
    }

    // 回溯：从终点倒推回起点
    if (distances.find(endId) == distances.end() || distances[endId] == std::numeric_limits<double>::infinity()) {
        return path; // 终点不可达
    }

    for (int at = endId; at != startId; at = previous[at]) {
        path.push_back(at);
    }
    path.push_back(startId);

    // 因为是倒推的，所以需要将数组翻转过来，变成 起点 -> 终点
    std::reverse(path.begin(), path.end());

    return path;
}

} // namespace core