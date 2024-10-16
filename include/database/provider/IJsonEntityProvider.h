#ifndef SPHINXD_IJSONENTITYPROVIDER_H
#define SPHINXD_IJSONENTITYPROVIDER_H

#include "./database/provider/JsonEntity.h"
#include <list>
#include <string>
#include <set>
#include <json/value.h>

class IJsonEntityProvider {
public:
    virtual ~IJsonEntityProvider() = default;

    virtual std::list<JsonEntity> read(const std::string& entityName, const Json::Value& filter, const std::set<std::string>& fields = {}) = 0;
};

#endif //SPHINXD_IJSONENTITYPROVIDER_H