#ifndef SPHINXD_IDATABASECONFIGURATOR_H
#define SPHINXD_IDATABASECONFIGURATOR_H

#include "ITypesConverter.h"
#include "ISchemeLoader.h"
#include "IDatabaseController.h"
#include "ICrudController.h"
#include "ISchemeUpdater.h"

class IDatabaseConfigurator {
public:
    virtual ~IDatabaseConfigurator() = default;
    virtual void configure(
            std::shared_ptr<ITypesConverter>& typesConverter,
            std::shared_ptr<ISchemeLoader>& schemeLoader,
            std::shared_ptr<IDatabaseController>& databaseController,
            std::shared_ptr<ICrudController>& crudController,
            std::shared_ptr<ISchemeUpdater>& schemeUpdater) = 0;
};

#endif //SPHINXD_IDATABASECONFIGURATOR_H
