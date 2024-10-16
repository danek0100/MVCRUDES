#ifndef SPHINXD_POSTGRESQLCONFIGURATOR_H
#define SPHINXD_POSTGRESQLCONFIGURATOR_H

#include <include/database/controllers/IDatabaseConfigurator.h>
#include <include/database/controllers/postgresql/PostgreSqlTypesConverter.h>
#include <include/database/controllers/postgresql/PostgreSqlSchemeLoader.h>
#include <include/database/controllers/postgresql/PostgreSqlDatabaseController.h>
#include <include/database/controllers/postgresql/PostgreSqlCrudController.h>
#include <include/database/controllers/postgresql/PostgreSqlSchemeUpdater.h>


class PostgreSqlConfigurator : public IDatabaseConfigurator {
public:
    void configure(
            std::shared_ptr<ITypesConverter>& typesConverter,
            std::shared_ptr<ISchemeLoader>& schemeLoader,
            std::shared_ptr<IDatabaseController>& databaseController,
            std::shared_ptr<ICrudController>& crudController,
            std::shared_ptr<ISchemeUpdater>& schemeUpdater) override
    {
        typesConverter = std::make_shared<PostgreSqlTypesConverter>();
        schemeLoader = std::make_shared<PostgreSqlSchemeLoader>(typesConverter);
        databaseController = std::make_shared<PostgreSqlDatabaseController>(typesConverter, EntitiesStructureCache::getSingletonInstance());
        crudController = std::make_shared<PostgreSqlCrudController>(typesConverter);
        schemeUpdater = std::make_shared<PostgreSqlSchemeUpdater>(typesConverter, databaseController);
    }
};

#endif //SPHINXD_POSTGRESQLCONFIGURATOR_H
