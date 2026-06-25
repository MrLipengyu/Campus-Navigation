#pragma once
#include <unordered_map>
#include <QString>
#include "Building.h"
#include "../graph/Graph.h"
#include "../../database/DatabaseManager.h" // 👇 引入数据库管理器

namespace core {

class CampusMap {
public:
    CampusMap() : m_dbManager("campus_data.db") {} // 初始化数据库路径
    ~CampusMap() = default;

    bool loadFromJson(const QString& filePath);

    const Graph& getGraph() const { return m_graph; }
    const Building* getBuilding(int id) const;
    const std::unordered_map<int, Building>& getAllBuildings() const;

    // 👇 新增：向外提供修改建筑信息的业务接口
    bool updateBuildingInfo(int id, const std::string& newInfo);

private:
    Graph m_graph;
    std::unordered_map<int, Building> m_buildings;

    // 👇 新增：让 CampusMap 长期持有数据库管理器
    database::DatabaseManager m_dbManager;
};

} // namespace core