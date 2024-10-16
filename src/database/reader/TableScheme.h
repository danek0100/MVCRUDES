#ifndef SPHINXD_TABLESCHEME_H
#define SPHINXD_TABLESCHEME_H

#include    "StructureField.h"
#include    "EntityScheme.h"
#include    "Relation.h"
#include    <string>
#include    <utility>
#include    <vector>
#include    <unordered_map>

/**
 * TableScheme represents the schema of a database table within the application.
 * It encapsulates the table's metadata, including its name, version, and structural information
 * such as fields, relations with other tables, indexes for efficient querying, and the database engines
 * compatible with this schema. The schema also specifies whether an external data provider is required
 * to populate the table with data. This class is used to define and manipulate the structure of tables
 * in a database-centric application, ensuring that the application's data model is consistent with the
 * underlying database structure.
 */
class TableScheme : public EntityScheme {
public:
    const std::vector<StructureField> fields;
    const std::unordered_map<std::string, std::vector<std::string>> indexes;
    const std::unordered_map<std::string, std::vector<std::string>> uniqueFieldsSets;
    const std::unordered_map<std::string, Relation> relations;
    const std::unordered_map<std::string, std::string> databaseEngines;

    TableScheme(const std::string& tableName,
                int tableVersion,
                bool requiresProvider,
                const std::vector<StructureField>& tableFields,
                const std::unordered_map<std::string, std::vector<std::string>>& tableIndexes,
                const std::unordered_map<std::string, std::vector<std::string>>& tableUniqueFieldsSets,
                const std::unordered_map<std::string, Relation>& tableRelations,
                const std::unordered_map<std::string, std::string>& supportedDatabaseEngines)
            : EntityScheme(tableName, tableVersion, requiresProvider),
              fields(tableFields), relations(tableRelations), indexes(tableIndexes), uniqueFieldsSets(tableUniqueFieldsSets), databaseEngines(supportedDatabaseEngines) {}
};

#endif //SPHINXD_TABLESCHEME_H
