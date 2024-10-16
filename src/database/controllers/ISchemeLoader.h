#ifndef SPHINXD_ISCHEMELOADER_H
#define SPHINXD_ISCHEMELOADER_H

#include    "./database/reader/TableScheme.h"
#include    "./sql/BaseDbClientProvider.h"
#include    <string>
#include    <vector>
#include    <unordered_map>
#include    <list>


class ISchemeLoader : public BaseDbClientProvider {
public:

    /**
     * Virtual destructor for safe polymorphic deletion.
     */
    virtual ~ISchemeLoader() = default;

    /**
     * Retrieves the actual schema of a table.
     *
     * @param tableName The name of the table for which the schema is to be retrieved.
     * @return The actual table scheme as currently defined in the database.
     */
    virtual TableScheme getActualScheme(const std::string& tableName) = 0;
};

#endif //SPHINXD_ISCHEMELOADER_H
