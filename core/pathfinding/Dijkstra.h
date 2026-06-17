#pragma once
#include <vector>
#include "../graph/Graph.h"

namespace core {

class Pathfinder {
public:
    // 构造函数传入图的常引用，保证寻路算法绝对不会修改图数据
    explicit Pathfinder(const Graph& graph);

    // 核心算法：返回从起点到终点的最短路径节点 ID 序列
    // 如果返回空的 vector，说明没有通路
    std::vector<int> findShortestPath(int startId, int endId);

private:
    const Graph& m_graph;
};

} // namespace core