#ifndef SPHINXD_POSTGRESQLDATABASECONTROLLER_H
#define SPHINXD_POSTGRESQLDATABASECONTROLLER_H

#include    "./database/JsonsConstans.h"
#include    "./database/provider/JsonEntityFilter.h"
#include    "./database/reader/StructureField.h"
#include    "./sql/SqlPool.h"
#include    <include/database/cache/IEntitiesStructureCache.h>
#include    <include/database/controllers/IDatabaseController.h>
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
 *
 * Key features include:
 * - Creation and deletion of databases and tables with PostgreSQL-specific optimizations.
 * - Version management for database tables to facilitate schema migrations.
 * - Detailed CRUD operations that leverage PostgreSQL's SQL capabilities for efficient data manipulation.
 * - Custom SQL query construction for advanced filtering and data retrieval.
 * - Adaptation of generic data types to PostgreSQL-specific types for seamless data integration.
 */
class PostgreSqlDatabaseController : public IDatabaseController {
public:
    PostgreSqlDatabaseController(std::shared_ptr<ITypesConverter> typesConverter, IEntitiesStructureCache* structures);
    ~PostgreSqlDatabaseController() override;

    // Databases
    void resetDatabases() override;
    void createDatabase(const std::string& dbName) override;
    void dropDatabase(const std::string& dbName) override;

    // Tables
    int createTable(const TableScheme& scheme) override;
    int dropTable(const TableScheme& scheme) override;
    int upsertTableVersion(const std::string& tableName, int version) override;

    int getEntityVersion(const std::string& tableName) override;

    int createIndexes(const std::unordered_map<std::string, std::vector<std::string>>& indexes, const std::string& tableName);
    int addUniqueConstraints(const std::unordered_map<std::string, std::vector<std::string>>& uniqueFieldsSets, const std::string& tableName);
    int buildRelations(const std::unordered_map<std::string, Relation>& relations, const std::string& tableName);

private:
    std::shared_ptr<ITypesConverter> typesConverter;
    IEntitiesStructureCache* structures;

    std::string generateFieldDefinition(const StructureField& field);

    int createEnums(const TableScheme& scheme);
    int dropEnums(const TableScheme& scheme);

    bool tableExists(const std::string& tableName);
    int getVersionFromVersionTable(const std::string& tableName);
    std::string buildFieldSetString(const std::vector<std::string>& fields, std::stringstream& nameStream);
};

#endif //SPHINXD_POSTGRESQLDATABASECONTROLLER_H
