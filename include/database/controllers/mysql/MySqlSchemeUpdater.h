#ifndef SPHINXD_MYSQLSCHEMEUPDATER_H
#define SPHINXD_MYSQLSCHEMEUPDATER_H

#include    "./database/JsonsConstans.h"
#include    "./database/cache/EntitiesStructureCache.h"
#include    "./database/reader/StructureField.h"
#include    "./sql/SqlPool.h"
#include    <include/database/controllers/ISchemeUpdater.h>
#include    <include/database/controllers/ITypesConverter.h>
#include    <include/database/controllers/IDatabaseController.h>
#include    <iostream>
#include    <list>
#include    <stdexcept>
#include    <string>
#include    <vector>
#include    <unordered_map>


class MySqlSchemeUpdater : public ISchemeUpdater {
private:
    std::shared_ptr<ITypesConverter> typesConverter;
    std::shared_ptr<IDatabaseController> databaseController;

    std::unordered_map<std::string, StructureField> mapFields(const std::vector<StructureField>& fields);

    void addNewFieldToTable(const std::string& tableName, const StructureField& field, std::vector<std::string>& scripts);

    void updateColumnProperties(const std::string& tableName, const StructureField& oldField,
                                const StructureField& newField, std::vector<std::string>& scripts);

public:
    MySqlSchemeUpdater(std::shared_ptr<ITypesConverter> typesConverter,
                       std::shared_ptr<IDatabaseController> databaseController);
    ~MySqlSchemeUpdater() override;

    int updateCurrentScheme(const TableScheme& currentScheme, const TableScheme& newScheme) override;
};

#endif //SPHINXD_MYSQLSCHEMEUPDATER_H
