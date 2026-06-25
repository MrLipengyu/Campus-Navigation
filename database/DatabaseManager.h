#pragma once
#include <QSqlDatabase>
#include <QString>
#include <vector>
#include "../core/map/Building.h"

namespace database {

// 采用经典的 DAO (Data Access Object) 模式
class DatabaseManager {
public:
    // 构造时传入数据库文件路径
    explicit DatabaseManager(const QString& dbPath = "campus_data.db");
    ~DatabaseManager();

    // 初始化数据库并建表
    bool initialize();

    // =============== CRUD 核心操作 ===============

    // 增：插入新的建筑信息
    bool insertBuilding(const core::Building& building);

    // 查：获取所有建筑信息
    std::vector<core::Building> getAllBuildings();

    // 查：根据 ID 获取单个建筑
    bool getBuildingById(int id, core::Building& outBuilding);

    // 改：更新建筑的简介和开放时间 (管理员功能)
    bool updateBuildingInfo(int id, const std::string& newInfo);

private:
    QSqlDatabase m_db;
    QString m_dbPath;
};

} // namespace database