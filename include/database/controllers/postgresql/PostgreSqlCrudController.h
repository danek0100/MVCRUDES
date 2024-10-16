#ifndef SPHINXD_POSTGRESQLCRUDCONTROLLER_H
#define SPHINXD_POSTGRESQLCRUDCONTROLLER_H

#include    "./database/JsonsConstans.h"
#include    "./database/cache/EntitiesStructureCache.h"
#include    "./database/provider/JsonEntityFilter.h"
#include    "./database/reader/StructureField.h"
#include    "./sql/SqlPool.h"
#include    <include/database/controllers/ICrudController.h>
#include    <include/database/controllers/ITypesConverter.h>
#include    <iostream>
#include    <list>
#include    <stdexcept>
#include    <string>
#include    <vector>
#include    <unordered_map>


/**
 * @class PostgreSqlDatabaseController
 *
 * @brief Implements the IDatabaseController interface for PostgreSQL databases.
 *
 * This class offers PostgreSQL-specific implementations for a variety of database operations required
 * by the IDatabaseController interface. It is designed to manage database and table creation and deletion,
 * table version management, schema updates, and supports detailed CRUD operations on table entities.
 * It utilizes PostgreSQL's unique features and capabilities to perform efficient data type mapping, value
 * adaptation, and query execution. Additionally, it provides methods for constructing complex SQL queries
 * based on abstract filter criteria, thereby simplifying the interaction with PostgreSQL databases while
 * maintaining the flexibility and power of SQL.
 * // TODO
 * Key features include:
 * - Creation and deletion of databases and tables with PostgreSQL-specific optimizations.
 * - Version management for database tables to facilitate schema migrations.
 * - Detailed CRUD operations that leverage PostgreSQL's SQL capabilities for efficient data manipulation.
 * - Custom SQL query construction for advanced filtering and data retrieval.
 * - Adaptation of generic data types to PostgreSQL-specific types for seamless data integration.
 */
class PostgreSqlCrudController : public ICrudController {
public:
    PostgreSqlCrudController(std::shared_ptr<ITypesConverter> typesConverter);
    ~PostgreSqlCrudController() override;

    // CRUD
    std::list<JsonEntity> loadEntities(const std::string& tableName, const Json::Value& filter, const std::set<std::string>& fields) override;
    void addEntities(std::list<JsonEntity>& objects) override;
    std::list<JsonEntity> updateEntities(std::list<JsonEntity>& objects) override;
    std::list<JsonEntity> removeEntities(std::list<JsonEntity>& objects) override;

private:
    std::shared_ptr<ITypesConverter> typesConverter;

    std::string buildSelectWhereClause(const std::string& entityName, const Json::Value& filter);
    std::string buildSelectOrderByClause(const Json::Value& filter);
    std::string join(const std::vector<std::string>& elements, const std::string& connector);
    std::string getSqlOperator(const std::string& type);
};

#endif //SPHINXD_POSTGRESQLCRUDCONTROLLER_H
