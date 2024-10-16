#ifndef SPHINXD_IDATABASECONTROLLER_H
#define SPHINXD_IDATABASECONTROLLER_H

#include    "./database/reader/TableScheme.h"
#include    "./sql/BaseDbClientProvider.h"
#include    <string>
#include    <vector>
#include    <unordered_map>
#include    <list>

/**
 * @class IDatabaseController
 *
 * @brief Interface for database operations.
 *
 * Provides an abstract interface for managing databases and their entities across different
 * database management systems (DBMS). This interface defines operations for creating and
 * dropping databases, managing table structures and their versions, performing CRUD (Create, // TODO
 * Read, Update, Delete) operations on entities, and adapting data types and values for
 * compatibility with specific DBMS. Implementations should handle errors gracefully.
 */
class IDatabaseController : public BaseDbClientProvider {
public:

    /**
     * Virtual destructor for safe polymorphic deletion.
     */
    virtual ~IDatabaseController() = default;

    /**
     * Recreate databases.
     */
    virtual void resetDatabases() = 0;

    /**
     * Creates a new database.
     *
     * @param dbName The name of the database to be created.
     */
    virtual void createDatabase(const std::string& dbName) = 0;

    /**
     * Drops an existing database.
     *
     * @param dbName The name of the database to be dropped.
     */
    virtual void dropDatabase(const std::string& dbName) = 0;

    /**
     * Creates a new table based on the provided schema.
     *
     * @param scheme The schema definition for the table, including fields, relationships, and indexes.
     * @return Integer status code: 0 on success, -1 on failure.
     */
    virtual int createTable(const TableScheme& scheme) = 0;

    /**
     * Drops an existing table.
     *
     * @param scheme The schema definition for the table, including fields, relationships, and indexes.
     * @return 0 if success, -1 otherwise.
     */
    virtual int dropTable(const TableScheme& scheme) = 0;

    /**
     * Inserts or updates the version of a table in the version control table.
     *
     * @param tableName The name of the table whose version is being updated.
     * @param version The new version number to be inserted or updated.
     * @return Integer status code indicating success (0) or failure (-1).
     */
    virtual int upsertTableVersion(const std::string& tableName, int version) = 0;

    /**
     * Retrieves the current version of a table from the version control table.
     *
     * @param tableName The name of the table whose version is to be retrieved.
     * @return The current version of the table, or -1 if the table does not exist or -2 if an error occurs.
     */
    virtual int getEntityVersion(const std::string& tableName) = 0;
};

#endif //SPHINXD_IDATABASECONTROLLER_H
