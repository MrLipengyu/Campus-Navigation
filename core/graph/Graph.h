#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace core {

// 前向声明
struct Edge;

// 地点节点
struct Node {
    int id;                 // 唯一标识符
    std::string name;       // 建筑/地点名称
    std::string info;       // 简介信息
    int x, y;               // 逻辑坐标 (用于2.5D视图计算)
    std::vector<Edge> edges; // 相邻的边 (邻接表的核心)
};

// 连接路径
struct Edge {
    int toNodeId;           // 目标节点的ID
    double distance;        // 物理距离 (权重)
    // int crowdedness;     // 拥挤度 (扩展功能预留：可用于计算最快路线而非最短路线)
};

// 图管理器
class Graph {
public:
    Graph() = default;
    ~Graph() = default;

    // 添加节点
    void addNode(int id, const std::string& name, const std::string& info, int x, int y);

    // 添加无向边 (调用一次，内部自动处理双向连接)
    void addUndirectedEdge(int fromId, int toId, double distance);

    // 获取特定节点 (如果不存在返回 nullptr)
    const Node* getNode(int id) const;

    // 获取所有节点 (用于UI层遍历绘制)
    const std::unordered_map<int, Node>& getAllNodes() const;

    // 清空图数据
    void clear();

private:
    std::unordered_map<int, Node> m_nodes;
};

} // namespace core