#pragma once
#include <vector>
#include "../graph/Graph.h"

namespace core {

// A* 寻路算法：在 Dijkstra 基础上引入启发函数 h(n)，
// 优先向目标方向探索，通常比 Dijkstra 更快找到最短路径。
// 启发函数采用欧几里得距离（两点间直线距离），保证可采纳性（不会高估实际距离）。
class AStarPathfinder {
public:
    explicit AStarPathfinder(const Graph& graph);

    // 接口与 Pathfinder::findShortestPath 完全一致，方便内部切换调用
    std::vector<int> findShortestPath(int startId, int endId);

private:
    // 启发函数 h(n)：节点 nodeId 到目标节点 goalId 的欧几里得距离
    // 因为地图边权重就是像素距离，欧几里得距离永远 <= 实际路径，满足可采纳性
    double heuristic(int nodeId, int goalId) const;

    const Graph& m_graph;
};

} // namespace core
