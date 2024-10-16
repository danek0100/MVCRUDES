#ifndef SPHINXD_MYSQLDATABASECONTROLLER_H
#define SPHINXD_MYSQLDATABASECONTROLLER_H

#include    "./database/JsonsConstans.h"
#include    "./database/cache/EntitiesStructureCache.h"
#include    "./database/provider/JsonEntityFilter.h"
#include    "./database/reader/StructureField.h"
#include    "./sql/SqlPool.h"
#include    <include/database/controllers/IDatabaseController.h>
#include    <include/database/controllers/ITypesConverter.h>
#include    <iostream>
#include    <list>
#include    <stdexcept>
#include    <string>
#include    <vector>
#include    <unordered_map>

/**
 * @class MySqlDatabaseController
 *
 * @brief Implements the IDatabaseController interface specifically for MySQL databases.
 *
 * This class offers MySQL-specific functionality for managing database operations. It encapsulates
 * the intricacies of MySQL, including database and table creation/deletion, schema management,
 * and performing CRUD (Create, Read, Update, Delete) operations. Special emphasis is placed on
 * MySQL's data type mapping, value adaptation, and efficient query execution. The controller
 * facilitates seamless integration of generic application data structures with MySQL's storage
 * and retrieval mechanisms, ensuring data integrity and performance optimization.
 *
 * Key functionalities include: // TODO
 * - MySQL-specific database and table lifecycle management.
 * - Advanced schema versioning to support schema migrations and updates.
 * - Comprehensive CRUD operations tailored for MySQL's optimization features.
 * - Dynamic query generation for complex filtering using the JSON-based filtering criteria.
 * - Custom data type mapping and value adaptation to bridge application data types with MySQL.
 */
class MySqlDatabaseController : public IDatabaseController {
public:
    MySqlDatabaseController(std::shared_ptr<ITypesConverter> typesConverter);
    ~MySqlDatabaseController() override;

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
    int addUniqueConstraints(const std::unordered_map<std::string, std::vector<std::string>>& uniqueFieldsSets, const std::vector<StructureField>& fields, const std::string& tableName);
    int buildRelations(const std::unordered_map<std::string, Relation>& relations, const std::string& tableName);
    int setTableEngine(const std::string& tableName, const std::string& engineName);

private:
    std::shared_ptr<ITypesConverter> typesConverter;

    std::vector<std::string> generateFieldTypeStrings(const TableScheme& scheme);
    void adjustFieldTypeStringsForRowSize(std::vector<std::string>& fieldTypeStrings);
    size_t calculateRowSizeFromStrings(const std::vector<std::string>& fieldTypeStrings);
    size_t calculateFieldSize(const std::string& typeName, int length);
    std::string generateFieldDefinition(const StructureField& field);
    bool tableExists(const std::string& tableName);
    int getVersionFromVersionTable(const std::string& tableName);

    static std::string fromKeyFieldsString(const std::vector<std::string>& fieldsSet, const std::vector<StructureField>& fields);
};

#endif //SPHINXD_MYSQLDATABASECONTROLLER_H
