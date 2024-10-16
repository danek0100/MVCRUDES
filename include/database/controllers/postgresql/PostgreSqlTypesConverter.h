#ifndef SPHINXD_POSTGRESQLTYPESCONVERTER_H
#define SPHINXD_POSTGRESQLTYPESCONVERTER_H

#include    "./database/JsonsConstans.h"
#include    "./database/cache/EntitiesStructureCache.h"
#include    "./database/provider/JsonEntityFilter.h"
#include    "./database/reader/StructureField.h"
#include    "./sql/SqlPool.h"
#include    <include/database/controllers/ITypesConverter.h>
#include    <iostream>
#include    <list>
#include    <stdexcept>
#include    <string>
#include    <vector>
#include    <unordered_map>

class PostgreSqlTypesConverter : public ITypesConverter {
private:
    std::shared_ptr<EntitiesStructureCache> entitiesStructures;

public:
    PostgreSqlTypesConverter();
    PostgreSqlTypesConverter(std::shared_ptr<EntitiesStructureCache> entitiesStructures);
    ~PostgreSqlTypesConverter() override;

    // Mappers
    std::string adaptValueToType(const std::string& typeName, const Json::Value& value) override;
    std::string getMappedType(const std::string& typeName, int length) override;
    std::string getUnmappedType(const std::string& typeString) override;
};

#endif //SPHINXD_POSTGRESQLTYPESCONVERTER_H
