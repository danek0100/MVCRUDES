#ifndef SPHINXD_ISCHEMEUPDATER_H
#define SPHINXD_ISCHEMEUPDATER_H

#include    "./database/reader/TableScheme.h"
#include    "./sql/BaseDbClientProvider.h"
#include    <string>
#include    <vector>
#include    <unordered_map>
#include    <list>


class ISchemeUpdater : public BaseDbClientProvider {
public:

    /**
     * Virtual destructor for safe polymorphic deletion.
     */
    virtual ~ISchemeUpdater() = default;

    /**
     * Updates the schema of an existing table to match a new schema.
     *
     * @param oldScheme The current scheme of the table.
     * @param newScheme The new scheme to which the table should be updated.
     * @return Integer status code indicating success (0) or failure (-1).
     */
    virtual int updateCurrentScheme(const TableScheme& oldScheme, const TableScheme& newScheme) = 0;
};

#endif //SPHINXD_ISCHEMELOADER_H
