#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>

namespace database {

DatabaseManager::DatabaseManager(const QString& dbPath) : m_dbPath(dbPath) {
    // 检查并添加 SQLite 驱动
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        m_db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
    }
    m_db.setDatabaseName(m_dbPath);
}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initialize() {
    if (!m_db.open()) {
        qWarning() << "数据库打开失败:" << m_db.lastError().text();
        return false;
    }

    // 创建 buildings 表（如果不存的话）
    QSqlQuery query;
    QString createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS buildings (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL,
            info TEXT,
            ui_x INTEGER,
            ui_y INTEGER,
            hitbox_radius INTEGER,
            entrance_node_id INTEGER
        )
    )";

    if (!query.exec(createTableSQL)) {
        qWarning() << "建表失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "[数据库] 初始化成功，连接到:" << m_dbPath;
    return true;
}

bool DatabaseManager::insertBuilding(const core::Building& b) {
    QSqlQuery query;
    // 使用预编译语句（Prepared Statement），防止 SQL 注入
    query.prepare("INSERT OR REPLACE INTO buildings (id, name, info, ui_x, ui_y, hitbox_radius, entrance_node_id) "
                  "VALUES (:id, :name, :info, :ui_x, :ui_y, :radius, :entrance_id)");

    query.bindValue(":id", b.id);
    query.bindValue(":name", QString::fromStdString(b.name));
    query.bindValue(":info", QString::fromStdString(b.info));
    query.bindValue(":ui_x", b.ui_x);
    query.bindValue(":ui_y", b.ui_y);
    query.bindValue(":radius", b.hitbox_radius);
    query.bindValue(":entrance_id", b.entrance_node_id);

    if (!query.exec()) {
        qWarning() << "插入数据失败:" << query.lastError().text();
        return false;
    }
    return true;
}

std::vector<core::Building> DatabaseManager::getAllBuildings() {
    std::vector<core::Building> buildings;
    QSqlQuery query("SELECT id, name, info, ui_x, ui_y, hitbox_radius, entrance_node_id FROM buildings");

    while (query.next()) {
        core::Building b;
        b.id = query.value(0).toInt();
        b.name = query.value(1).toString().toStdString();
        b.info = query.value(2).toString().toStdString();
        b.ui_x = query.value(3).toInt();
        b.ui_y = query.value(4).toInt();
        b.hitbox_radius = query.value(5).toInt();
        b.entrance_node_id = query.value(6).toInt();
        buildings.push_back(b);
    }
    return buildings;
}

bool DatabaseManager::getBuildingById(int id, core::Building& outBuilding) {
    QSqlQuery query;
    query.prepare("SELECT name, info, ui_x, ui_y, hitbox_radius, entrance_node_id FROM buildings WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        outBuilding.id = id;
        outBuilding.name = query.value(0).toString().toStdString();
        outBuilding.info = query.value(1).toString().toStdString();
        outBuilding.ui_x = query.value(2).toInt();
        outBuilding.ui_y = query.value(3).toInt();
        outBuilding.hitbox_radius = query.value(4).toInt();
        outBuilding.entrance_node_id = query.value(5).toInt();
        return true;
    }
    return false;
}

bool DatabaseManager::updateBuildingInfo(int id, const std::string& newInfo) {
    QSqlQuery query;
    query.prepare("UPDATE buildings SET info = :info WHERE id = :id");
    query.bindValue(":info", QString::fromStdString(newInfo));
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "更新数据失败:" << query.lastError().text();
        return false;
    }
    return true;
}

} // namespace database