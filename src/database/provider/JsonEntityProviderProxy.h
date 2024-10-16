#ifndef SPHINXD_JSONENTITYPROVIDERPROXY_H
#define SPHINXD_JSONENTITYPROVIDERPROXY_H

#include "JsonEntityProvider.h"
#include <include/database/provider/IJsonEntityProvider.h>

class JsonEntityProviderProxy : public IJsonEntityProvider {
public:
    static std::shared_ptr<JsonEntityProviderProxy> get() {
        static std::shared_ptr<JsonEntityProviderProxy> instance(new JsonEntityProviderProxy());
        return instance;
    }

    std::list<JsonEntity> read(const std::string& entityName, const Json::Value& filter, const std::set<std::string>& fields = {}) override {
        return JsonEntityProvider::getSingletonInstance()->read(entityName, filter, fields);
    }
};

#endif //SPHINXD_JSONENTITYPROVIDERPROXY_H