#include    "DatabaseManager.h"

namespace fs = std::experimental::filesystem;

std::shared_ptr<IDatabaseController> DatabaseManager::databaseController = nullptr;
std::shared_ptr<ICrudController> DatabaseManager::crudController = nullptr;
std::shared_ptr<ISchemeLoader> DatabaseManager::schemeLoader = nullptr;
std::shared_ptr<ISchemeUpdater> DatabaseManager::schemeUpdater = nullptr;
std::shared_ptr<ITypesConverter> DatabaseManager::typesConverter = nullptr;

std::map<int, std::shared_ptr<IDatabaseConfigurator>> DatabaseManager::configurators = {
        {DBSERVERTYPE_INTERNAL, std::make_shared<MySqlConfigurator>()},
        {DBSERVERTYPE_MYSQL,  std::make_shared<MySqlConfigurator>()},
        {DBSERVERTYPE_POSTGRES, std::make_shared<PostgreSqlConfigurator>()}
};

DatabaseManager::DatabaseManager() {
    initComponents();
}

std::shared_ptr<IDatabaseController>& DatabaseManager::getDatabaseController() {
    if (!databaseController) {
        initComponents();
    }
    return databaseController;
}

std::shared_ptr<ICrudController>& DatabaseManager::getCrudController() {
    if (!crudController) {
        initComponents();
    }
    return crudController;
}

std::shared_ptr<ISchemeLoader>& DatabaseManager::getSchemeLoader() {
    if (!schemeLoader) {
        initComponents();
    }
    return schemeLoader;
}

std::shared_ptr<ISchemeUpdater>& DatabaseManager::getSchemeUpdater() {
    if (!schemeUpdater) {
        initComponents();
    }
    return schemeUpdater;
}

void DatabaseManager::initComponents() {
    auto dbType = DbUsersConfig::get()->getDbServerType();
    auto it = configurators.find(dbType);
    if (it != configurators.end()) {
        it->second->configure(typesConverter, schemeLoader, databaseController, crudController, schemeUpdater);
    } else {
        throw std::runtime_error("Unsupported DB server type");
    }
}

int DatabaseManager::resetDatabases() {
    TRACE(__FUNCTION__ << "() entry point.");
    databaseController->resetDatabases();
    TRACE(__FUNCTION__ << "() end point.");
    return 0;
}

int DatabaseManager::resetTables() {
    TRACE(__FUNCTION__ << "() entry point.");

    std::string entitiesPath = SchemeReader::getSingletonInstance()->getDefaultJsonsDirectoryPath() + "/entities";
    auto parsedSchemes = SchemeReader::getSingletonInstance()->readSchemes(entitiesPath);

    int result = createVersionsTable();
    if (result != 0) {
        return result;
    };

    for (const auto& scheme : parsedSchemes) {
        if (scheme->name == VERSIONS_TABLE_NAME) {
            continue;
        }
        if (scheme->version > 0) {
            if (const auto* tableScheme = dynamic_cast<const TableScheme*>(scheme.get())) {
                if (initTable(*tableScheme) != 0) {
                    TRACE(__FUNCTION__ << "() Error! Cannot initialize table for " << scheme->name);
                    return -3;
                };
            }
        }
    }

    std::string defaultsPath = SchemeReader::getSingletonInstance()->getDefaultJsonsDirectoryPath() + "/defaults";
    auto parsedEntities = SchemeReader::getSingletonInstance()->readAllEntities(defaultsPath);

    if (!databaseController) {
        TRACE(__FUNCTION__ << "Error: Failed to create database instance.");
        return -3;
    }

    for (auto& entities : parsedEntities) {
        crudController->addEntities(entities.second);
    }

    TRACE(__FUNCTION__ << "() end point.");
    return 0;
}

int DatabaseManager::initTable(const TableScheme& scheme) {
    TRACE(__FUNCTION__ << "() entry point.");
    if (!databaseController) {
        TRACE(__FUNCTION__ << "Error: Failed to create database instance of " << scheme.name << ".");
        return -3;
    }

    databaseController->dropTable(scheme);
    int code = databaseController->createTable(scheme);
    if (code != 0) {
        return code;
    }
    code = databaseController->upsertTableVersion(scheme.name, scheme.version);

    TRACE(__FUNCTION__ << "() end point.");
    return code;
}

int DatabaseManager::update(int currentDbVersion) {
    return DbUpdater::getSingletonInstance()->update(currentDbVersion);
}

int DatabaseManager::createVersionsTable() {
    TRACE(__FUNCTION__ << "() entry point.");
    std::string entitiesPath = SchemeReader::getSingletonInstance()->getDefaultJsonsDirectoryPath() + "/entities";
    auto parsedEntities = SchemeReader::getSingletonInstance()->readSchemes(entitiesPath);

    auto versionsSchemeIt = std::find_if(parsedEntities.begin(), parsedEntities.end(),
                           [](const auto& entity) {
                               return entity->name == VERSIONS_TABLE_NAME;
                           });

    if (versionsSchemeIt == parsedEntities.end()) {
        TRACE(__FUNCTION__ << "() Error! " << VERSIONS_TABLE_NAME << " doesn't exists.");
        return -1;
    }
    const auto* versionsTable = dynamic_cast<const TableScheme*>(versionsSchemeIt->get());
    ASSERT(versionsTable)

    if (initTable(*versionsTable) != 0) {
        TRACE(__FUNCTION__ << "() Error! " << VERSIONS_TABLE_NAME << " cannot be created.");
        return -2;
    }
    TRACE(__FUNCTION__ << "() end point.");
    return 0;
}

int DatabaseManager::addDefaults(const std::list<std::string>& entitiesNames) {
    TRACE(__FUNCTION__ << "() entry point.");
    std::string defaultsPath = SchemeReader::getSingletonInstance()->getDefaultJsonsDirectoryPath() + "/defaults";
    auto parsedEntities = SchemeReader::getSingletonInstance()->readAllEntities(defaultsPath);

    for (const auto& entityName : entitiesNames) {
        auto it = parsedEntities.find(entityName);
        if (it != parsedEntities.end()) {
            crudController->addEntities(it->second); // Catch exception if will be needed error code
        }
    }
    TRACE(__FUNCTION__ << "() end point.");
    return 0;
}
