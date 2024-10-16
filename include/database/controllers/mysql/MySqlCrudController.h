#ifndef SPHINXD_MYSQLCRUDCONTROLLER_H
#define SPHINXD_MYSQLCRUDCONTROLLER_H

#include    "./database/JsonsConstans.h"
#include    "./database/cache/EntitiesStructureCache.h"
#include    "./database/provider/JsonEntityFilter.h"
#include    "./database/reader/StructureField.h"
#include    "./sql/SqlPool.h"
#include    <include/database/controllers/ICrudController.h>
#include    <include/database/controllers/ITypesConverter.h>
#include    <iostream>
#include    <list>
#include    <stdexcept>
#include    <string>
#include    <vector>
#include    <unordered_map>

class MySqlCrudController : public ICrudController {
public:
    MySqlCrudController(std::shared_ptr<ITypesConverter> typesConverter);
    ~MySqlCrudController() override;

    // CRUD
    std::list<JsonEntity> loadEntities(const std::string& tableName, const Json::Value& filter, const std::set<std::string>& fields) override;
    void addEntities(std::list<JsonEntity>& objects) override;
    std::list<JsonEntity> updateEntities(std::list<JsonEntity>& objects) override;
    std::list<JsonEntity> removeEntities(std::list<JsonEntity>& objects) override;

private:
    std::shared_ptr<ITypesConverter> typesConverter;

    std::string buildSelectWhereClause(const std::string& entityName, const Json::Value& filter);
    std::string buildSelectOrderByClause(const Json::Value &filter);
    std::string join(const std::vector<std::string>& elements, const std::string& connector);
    std::string getSqlOperator(const std::string& type);
};

#endif //SPHINXD_MYSQLCRUDCONTROLLER_H
