#pragma once
#include <unordered_map>
#include <QString>
#include "Building.h"
#include "../graph/Graph.h"

namespace core {

class CampusMap {
public:
    CampusMap() = default;
    ~CampusMap() = default;

    // 从 JSON 文件加载数据
    bool loadFromJson(const QString& filePath);

    // 获取底层图结构（供 Dijkstra 和 MapView 使用）
    const Graph& getGraph() const { return m_graph; }

    // 业务查询接口
    const Building* getBuilding(int id) const;
    const std::unordered_map<int, Building>& getAllBuildings() const;

private:
    Graph m_graph;
    std::unordered_map<int, Building> m_buildings;
};

} // namespace core