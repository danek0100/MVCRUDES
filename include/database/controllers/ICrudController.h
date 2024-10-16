#ifndef SPHINXD_ICRUDCONTROLLER_H
#define SPHINXD_ICRUDCONTROLLER_H

#include    "./database/provider/JsonEntity.h"
#include    "./sql/BaseDbClientProvider.h"
#include    <string>
#include    <vector>
#include    <unordered_map>
#include    <list>

class ICrudController : public BaseDbClientProvider {
public:

    /**
     * Virtual destructor for safe polymorphic deletion.
     */
    virtual ~ICrudController() = default;

    /**
     * Loads all entities from a specified table.
     *
     * @param tableName The name of the table from which to load entities.
     * @param filter The filter rule for loading process.
     * @param fields Fields to load at entity.
     * @return A list of entities loaded from the table.
     */
    virtual std::list<JsonEntity> loadEntities(const std::string& tableName, const Json::Value& filter, const std::set<std::string>& fields = {}) = 0;

    /**
     * Adds a list of new entities to a table.
     *
     * This method is used for inserting multiple entities into a table at once. The entities are added as is, without modifications.
     *
     * @param objects The list of entities to be added.
     */
    virtual void addEntities(std::list<JsonEntity>& objects) = 0;

    /**
     * Updates an existing list of entities in a table.
     *
     * @param objects The list of entities to be updated.
     * @return A list of entities after the update.
     */
    virtual std::list<JsonEntity> updateEntities(std::list<JsonEntity>& objects) = 0;

    /**
     * Removes a list of entities from a table.
     *
     * @param objects The list of entities to be removed.
     */
    virtual std::list<JsonEntity> removeEntities(std::list<JsonEntity>& objects) = 0;
};

#endif //SPHINXD_ICRUDCONTROLLER_H
