#ifndef SPHINXD_MYSQLTYPESCONVERTER_H
#define SPHINXD_MYSQLTYPESCONVERTER_H

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

class MySqlTypesConverter : public ITypesConverter {
private:
    std::shared_ptr<EntitiesStructureCache> entitiesStructures;

public:
    MySqlTypesConverter();
    MySqlTypesConverter(std::shared_ptr<EntitiesStructureCache> entitiesStructures);
    ~MySqlTypesConverter() override;

    // Mappers
    std::string adaptValueToType(const std::string& typeName, const Json::Value& value) override;
    std::string getMappedType(const std::string& typeName, int length) override;
    std::string getUnmappedType(const std::string& typeString) override;
};

#endif //SPHINXD_MYSQLTYPESCONVERTER_H
