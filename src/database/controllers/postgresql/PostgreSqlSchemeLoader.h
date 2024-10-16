#ifndef SPHINXD_POSTGRESQLSCHEMELOADER_H
#define SPHINXD_POSTGRESQLSCHEMELOADER_H

#include    "./database/JsonsConstans.h"
#include    "./database/reader/StructureField.h"
#include    "./sql/SqlPool.h"
#include    <include/database/cache/IEntitiesStructureCache.h>
#include    <include/database/controllers/ISchemeLoader.h>
#include    <include/database/controllers/ITypesConverter.h>
#include    <iostream>
#include    <list>
#include    <stdexcept>
#include    <string>
#include    <vector>
#include    <unordered_map>


class PostgreSqlSchemeLoader : public ISchemeLoader {
private:
    std::shared_ptr<ITypesConverter> typesConverter;
    std::shared_ptr<IEntitiesStructureCache> structures;

    std::vector<StructureField> loadFields(const std::string& tableName);
    StructureField getStructureFieldFormSqlResult(ISqlResult& sqlResult);
    std::unordered_map<std::string, std::vector<std::string>> loadIndexes(const std::string& tableName);
    std::unordered_map<std::string, std::vector<std::string>> loadUniqueSets(const std::string& tableName);
    std::unordered_map<std::string, Relation> loadRelations(const std::string& tableName);
    std::string getPostgresAction(const std::string& action);

public:
    PostgreSqlSchemeLoader(std::shared_ptr<ITypesConverter> typesConverter);
    PostgreSqlSchemeLoader(std::shared_ptr<ITypesConverter> typesConverter, std::shared_ptr<IEntitiesStructureCache> structures);
    ~PostgreSqlSchemeLoader() override;

    TableScheme getActualScheme(const std::string& tableName) override;
};

#endif //SPHINXD_POSTGRESQLSCHEMELOADER_H
