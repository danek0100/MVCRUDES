#ifndef SPHINXD_POSTGRESQLSCHEMEUPDATER_H
#define SPHINXD_POSTGRESQLSCHEMEUPDATER_H

#include    "./database/JsonsConstans.h"
#include    "./database/cache/EntitiesStructureCache.h"
#include    "./database/reader/StructureField.h"
#include    "./sql/SqlPool.h"
#include    <include/database/controllers/ISchemeUpdater.h>
#include    <include/database/controllers/IDatabaseController.h>
#include    <include/database/controllers/ITypesConverter.h>
#include    <iostream>
#include    <list>
#include    <stdexcept>
#include    <string>
#include    <vector>
#include    <unordered_map>


class PostgreSqlSchemeUpdater : public ISchemeUpdater {
private:
    std::shared_ptr<ITypesConverter> typesConverter;
    std::shared_ptr<IDatabaseController> databaseController;

    void addNewFieldToTable(const std::string& tableName, const StructureField& newField, std::vector<std::string>& scripts);

    void updateFieldType(const std::string& tableName, const StructureField& oldField,
                         const StructureField& newField, std::vector<std::string>& scripts);

    void updateFieldDefault(const std::string& tableName, const StructureField& oldField,
                            const StructureField& newField, std::vector<std::string>& scripts);

    void updateFieldNullability(const std::string& tableName, const StructureField& oldField,
                                const StructureField& newField, std::vector<std::string>& scripts);

    void updateUniqueConstraint(const std::string& tableName, const StructureField& oldField,
                                const StructureField& newField, std::vector<std::string>& scripts);

public:
    PostgreSqlSchemeUpdater(std::shared_ptr<ITypesConverter> typesConverter,
                            std::shared_ptr<IDatabaseController> databaseController);
    ~PostgreSqlSchemeUpdater() override;

    int updateCurrentScheme(const TableScheme& currentScheme, const TableScheme& newScheme) override;
};

#endif //SPHINXD_POSTGRESQLSCHEMEUPDATER_H
