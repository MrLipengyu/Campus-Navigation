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

    // 使用 find() 而非 operator[] 进行安全回溯。
    // operator[] 在键不存在时会自动插入默认值 0，可能导致死循环；
    // find() 在找不到前驱时返回 end()，可以安全中止并返回空路径。
    for (int at = endId; at != startId; ) {
        path.push_back(at);
        auto it = previous.find(at);
        if (it == previous.end()) {
            // 回溯链断裂（起点与终点实际不连通，防御性处理）
            path.clear();
            return path;
        }
        at = it->second;
    }
    path.push_back(startId);

    // 因为是倒推的，所以需要将数组翻转过来，变成 起点 -> 终点
    std::reverse(path.begin(), path.end());

    return path;
}

// 辅助函数：计算路线的总距离
double Pathfinder::calculatePathDistance(const std::vector<int>& path) {
    double dist = 0.0;
    for (size_t i = 0; i < path.size() - 1; ++i) {
        const Node* node = m_graph.getNode(path[i]);
        if (node) {
            for (const auto& edge : node->edges) {
                if (edge.toNodeId == path[i+1]) {
                    dist += edge.distance;
                    break;
                }
            }
        }
    }
    return dist;
}

// 🌟 核心扩展：TSP 近似算法 (贪心策略)
std::vector<int> Pathfinder::findTSPPath(int startId, std::vector<int> destIds, std::vector<int>& outVisitOrder) {
    std::vector<int> fullPath;
    outVisitOrder.clear();

    if (destIds.empty()) return fullPath;

    int currentStart = startId;
    outVisitOrder.push_back(currentStart); // 记录起点
    fullPath.push_back(currentStart);      // 初始化总路径

    // 当还有目的地没去过时，持续循环
    while (!destIds.empty()) {
        int bestDestIdx = -1;
        double minDistance = std::numeric_limits<double>::infinity();
        std::vector<int> bestSubPath;

        // 遍历所有尚未访问的目的地，找到距离当前位置最近的
        for (size_t i = 0; i < destIds.size(); ++i) {
            // 调用我们写好的 Dijkstra 算法计算当前点到该候选点的路径
            std::vector<int> subPath = findShortestPath(currentStart, destIds[i]);

            if (subPath.empty()) continue; // 如果根本走不通，跳过

            double dist = calculatePathDistance(subPath);
            if (dist < minDistance) {
                minDistance = dist;
                bestDestIdx = i;
                bestSubPath = subPath;
            }
        }

        // 如果所有剩余点都不可达，提前结束
        if (bestDestIdx == -1) {
            break;
        }

        int nextDest = destIds[bestDestIdx];
        outVisitOrder.push_back(nextDest); // 记录访问顺序

        // 拼接路径：跳过 subPath 的第一个点（因为它等于 currentStart，防止路径重复）
        for (size_t i = 1; i < bestSubPath.size(); ++i) {
            fullPath.push_back(bestSubPath[i]);
        }

        // 走到新的点，更新当前位置
        currentStart = nextDest;
        // 从未访问列表中移除这个点
        destIds.erase(destIds.begin() + bestDestIdx);
    }

    return fullPath;
}

} // namespace core