#include    "EntitiesStructureCache.h"
#include    "./database/reader/SchemeReader.h"

namespace fs = std::experimental::filesystem;

EntitiesStructureCache::EntitiesStructureCache() : Singleton<EntitiesStructureCache>() {}

void EntitiesStructureCache::loadCache(const std::string& directoryPath) {
    TRACE(__FUNCTION__ << "(). Entry point.");
    if (loadedPath != directoryPath) {
        clear();
        for (const auto& scheme : SchemeReader::getSingletonInstance()->readSchemes(directoryPath)) {
            providerCache[scheme->name] = scheme->needProvider;
            versionsCache[scheme->name] = scheme->version;

            if (const auto* enumTypeScheme = dynamic_cast<const EnumTypeScheme*>(scheme.get())) {
                enumTypesValuesCache[scheme->name] = enumTypeScheme->values;
            } else if (const auto* tableScheme = dynamic_cast<const TableScheme*>(scheme.get())) {
                fieldsCache[scheme->name] = tableScheme->fields;
                typesCache[scheme->name] = {};
                for (const auto& field : tableScheme->fields) {
                    typesCache.at(scheme->name)[field.get(FIELD_NAME).asString()] = field.get(FIELD_TYPE).asString();
                }
            }
        }
        loadedPath = directoryPath;
    }
    TRACE(__FUNCTION__ << "(). End point.");
}

bool EntitiesStructureCache::isTypeEnum(const std::string &key) {
    auto it = enumTypesValuesCache.find(key);
    return it != enumTypesValuesCache.end();
}

std::vector<std::string> EntitiesStructureCache::getEnumValues(const std::string &key) {
    auto it = enumTypesValuesCache.find(key);
    if (it != enumTypesValuesCache.end()) {
        return it->second;
    }
    return {};
}

std::vector<StructureField> EntitiesStructureCache::getFields(const std::string &key) {
    auto it = fieldsCache.find(key);
    if (it != fieldsCache.end()) {
        return it->second;
    }
    return {};
}

std::unordered_map<std::string, std::string> EntitiesStructureCache::getFieldsTypes(const std::string &key) {
    auto it = typesCache.find(key);
    if (it != typesCache.end()) {
        return it->second;
    }
    return {};
}

std::vector<std::string> EntitiesStructureCache::getKeys() {
    std::vector<std::string> keys;
    keys.reserve(fieldsCache.size());
    for (const auto& entity : fieldsCache) {
        keys.push_back(entity.first);
    }
    return keys;
}

bool EntitiesStructureCache::isNeedProvider(const std::string& entityName) {
    if (providerCache.find(entityName) != providerCache.end()) {
        return providerCache.at(entityName);
    }
    return false;
}

int EntitiesStructureCache::getVersion(const std::string& entityName) {
    if (versionsCache.find(entityName) != versionsCache.end()) {
        return versionsCache.at(entityName);
    }
    return 0;
}

void EntitiesStructureCache::clear() {
    fieldsCache.clear();
    providerCache.clear();
    versionsCache.clear();
    typesCache.clear();
}
