#ifndef SPHINXD_MYSQLCONFIGURATOR_H
#define SPHINXD_MYSQLCONFIGURATOR_H

#include <include/database/controllers/IDatabaseConfigurator.h>
#include <include/database/controllers/mysql/MySqlTypesConverter.h>
#include <include/database/controllers/mysql/MySqlSchemeLoader.h>
#include <include/database/controllers/mysql/MySqlDatabaseController.h>
#include <include/database/controllers/mysql/MySqlCrudController.h>
#include <include/database/controllers/mysql/MySqlSchemeUpdater.h>


class MySqlConfigurator : public IDatabaseConfigurator {
public:
    void configure(
            std::shared_ptr<ITypesConverter>& typesConverter,
            std::shared_ptr<ISchemeLoader>& schemeLoader,
            std::shared_ptr<IDatabaseController>& databaseController,
            std::shared_ptr<ICrudController>& crudController,
            std::shared_ptr<ISchemeUpdater>& schemeUpdater) override
    {
        typesConverter = std::make_shared<MySqlTypesConverter>();
        schemeLoader = std::make_shared<MySqlSchemeLoader>(typesConverter);
        databaseController = std::make_shared<MySqlDatabaseController>(typesConverter);
        crudController = std::make_shared<MySqlCrudController>(typesConverter);
        schemeUpdater = std::make_shared<MySqlSchemeUpdater>(typesConverter, databaseController);
    }
};

#endif //SPHINXD_MYSQLCONFIGURATOR_H
