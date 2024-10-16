#ifndef SPHINXD_DATABASEMANAGER_H
#define SPHINXD_DATABASEMANAGER_H

#include    "./database/JsonsConstans.h"
#include    "./database/reader/SchemeReader.h"
#include    "./database/update/DbUpdater.h"
#include    <include/database/controllers/mysql/MySqlConfigurator.h>
#include    <include/database/controllers/postgresql/PostgreSqlConfigurator.h>
#include    <DbUsersConfig.h>
#include    <algorithm>
#include    <string>
#include    <vector>
#include    <utility>
#include    <utils/Singleton.h>
#include    <unordered_map>


/**
 * @class DatabaseManager
 *
 * @brief Manages the initialization and setup of database tables using JSON configurations.
 *
 * Responsible for managing database tables based on JSON file definitions. This class handles
 * the creation and updating of tables, insertion of default values, and extraction of table
 * structures, indexes, relations, and database engines from JSON files. It supports the creation
 * and management of tables in a variety of database systems through the IDatabaseController interface.
 */
class DatabaseManager : public Singleton<DatabaseManager> {
private:
    static std::map<int, std::shared_ptr<IDatabaseConfigurator>> configurators;

    static std::shared_ptr<IDatabaseController> databaseController; // Instance of the database;
    static std::shared_ptr<ICrudController> crudController; // Instance of the crud;
    static std::shared_ptr<ISchemeLoader> schemeLoader; // Instance of the scheme loader;
    static std::shared_ptr<ISchemeUpdater> schemeUpdater; // Instance of the scheme updater;
    static std::shared_ptr<ITypesConverter> typesConverter; // Instance of the type converter;

    /**
     * Constructs a DatabaseManager with the specified path to JSON configurations.
     */
    DatabaseManager();

    static void initComponents();

public:
    friend class Singleton<DatabaseManager>;

    // Creates and return instance of IDatabaseController based on the database type.
    static std::shared_ptr<IDatabaseController>& getDatabaseController();

    // Creates and return instance of ICrudController based on the database type.
    static std::shared_ptr<ICrudController>& getCrudController();

    // Creates and return instance of ISchemeLoader based on the database type.
    static std::shared_ptr<ISchemeLoader>& getSchemeLoader();

    // Creates and return instance of ISchemeUpdater based on the database type.
    static std::shared_ptr<ISchemeUpdater>& getSchemeUpdater();

    /**
     * Resets and initializes databases.
     * This includes creating DBMS-wide types and structures.
     *
     * @return Integer status code indicating success (0) or various failure modes with negative values.
     */
    int resetDatabases();

    /**
     * Resets and initializes database tables based on JSON schema definitions.
     * This includes creating new tables and setting up their structures as per the definitions.
     *
     * @return Integer status code indicating success (0) or various failure modes with negative values.
     */
    int resetTables();

    /**
     * Initializes a single table in the database as per the provided schema.
     * This includes dropping the existing table (if any), creating a new table, and inserting its version.
     *
     * @param scheme The schema definition of the table to initialize.
     * @return Integer status code indicating success (0) or failure (-3).
     */
    int initTable(const TableScheme& scheme);

    /**
     * Applies updates to the database structures based on the provided current database version.
     * This includes both schema updates and data migration as needed.
     *
     * @param currentDbVersion The current version of the database to update from.
     * @return Integer status code indicating success (0) or failure (-1).
     */
    int update(int currentDbVersion);

    /**
     * Creates a special "versions" table in the database if it doesn't exist.
     * This table tracks the versions of other tables to manage schema updates.
     *
     * @return Integer status code indicating success (0) or various failure modes (-1, -2).
     */
    int createVersionsTable();

    /**
     * Populates tables with default values based on JSON definitions.
     * This is typically used after table creation to insert initial data.
     *
     * @param entitiesNames A list of entity names for which to add default values.
     * @return Integer status code indicating success (0).
     */
    int addDefaults(const std::list<std::string>& entitiesNames);
};

#endif //SPHINXD_DATABASEMANAGER_H
