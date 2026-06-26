#pragma once
#include <vector>
#include "../graph/Graph.h"

namespace core {

class Pathfinder {
public:
    explicit Pathfinder(const Graph& graph);

    // 原有的单点最短路径
    std::vector<int> findShortestPath(int startId, int endId);

    // 👇 新增：多终点近似 TSP 算法 (贪心最近邻)
    // destIds: 多个目标路网节点的 ID
    // outVisitOrder: 传出参数，用于返回算法实际计算出的访问顺序
    std::vector<int> findTSPPath(int startId, std::vector<int> destIds, std::vector<int>& outVisitOrder);

private:
    // 👇 新增辅助方法：计算某一段路径的真实物理长度
    double calculatePathDistance(const std::vector<int>& path);

    const Graph& m_graph;
};

} // namespace core