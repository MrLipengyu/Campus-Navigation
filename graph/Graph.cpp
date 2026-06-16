#include "Graph.h"

namespace core {

void Graph::addNode(int id, const std::string& name, const std::string& info, int x, int y) {
    // 只有当节点不存在时才添加
    if (m_nodes.find(id) == m_nodes.end()) {
        m_nodes[id] = {id, name, info, x, y, {}};
    }
}

void Graph::addUndirectedEdge(int fromId, int toId, double distance) {
    // 确保两个节点都存在
    if (m_nodes.find(fromId) != m_nodes.end() && m_nodes.find(toId) != m_nodes.end()) {
        // 添加 A -> B
        m_nodes[fromId].edges.push_back({toId, distance});
        // 添加 B -> A (无向图特性)
        m_nodes[toId].edges.push_back({fromId, distance});
    }
}

const Node* Graph::getNode(int id) const {
    //auto超长容器迭代器
    auto it = m_nodes.find(id);
    if (it != m_nodes.end()) {
        return &(it->second);
    }
    return nullptr; // 没找到
}

const std::unordered_map<int, Node>& Graph::getAllNodes() const {
    return m_nodes;
}

void Graph::clear() {
    m_nodes.clear();
}

} // namespace core