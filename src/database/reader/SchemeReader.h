#ifndef SPHINXD_ENTITIESREADER_H
#define SPHINXD_ENTITIESREADER_H

#include    "EnumTypeScheme.h"
#include    "TableScheme.h"
#include    "Relation.h"
#include    "./database/JsonsConstans.h"
#include    "./database/provider/JsonEntity.h"
#include    "./json/json.h"
#include    <experimental/filesystem>
#include    <list>
#include    <utils/Singleton.h>
#include    <unordered_map>

namespace fs = std::experimental::filesystem;

/**
 * @class SchemeReader
 *
 * @brief Responsible for reading and interpreting the structure of entities from JSON files.
 *
 * SchemeReader loads and parses JSON files that define the schemas of database tables and entities, including
 * their fields, relationships, indexes, and default values for entities. It utilizes a cache mechanism to optimize
 * reading processes and reduce redundant file access. As a part of the Singleton pattern, it ensures a single instance
 * manages schema and entity information throughout the application's lifecycle.
 */
class SchemeReader : public Singleton<SchemeReader> {
private:
    std::string jsonsDirectoryPath;

    SchemeReader();

    std::unordered_map<std::string, std::list<std::shared_ptr<EntityScheme>>> schemaCache;

    std::shared_ptr<EnumTypeScheme> readEnumTypeScheme(const fs::path& filePath);
    std::shared_ptr<TableScheme> readTableScheme(const fs::path& filePath);

    static bool isJsonFile(const fs::path& path);
    bool validateAndExtractName(const Json::Value& jsonRoot, std::string& name);
    bool parseJsonFile(const std::string& filePath, Json::Value& jsonRoot);

    bool processEntityFields(const Json::Value& jsonRoot, std::vector<StructureField>& fields);
    bool processField(const Json::Value& field, std::vector<StructureField>& fields);
    bool processEntityValues(const Json::Value& jsonRoot, std::vector<std::string>& values);
    void addFieldProperty(StructureField& field, const Json::Value& jsonField, const std::string& propertyName, const std::string& type);
    bool extractIndexes(const Json::Value& jsonRoot, std::unordered_map<std::string, std::vector<std::string>>& indexes, const std::string& schemeName);
    bool extractUniqueSets(const Json::Value& jsonRoot, std::unordered_map<std::string, std::vector<std::string>>& uniqueSets, const std::string& schemeName);
    bool extractRelations(const Json::Value& jsonRoot, std::unordered_map<std::string, Relation>& relations, const std::string& schemeName);
    bool extractDatabaseEngines(const Json::Value& jsonRoot, std::unordered_map<std::string, std::string>& engines);

    bool extractEntities(const Json::Value& jsonRoot, std::list<JsonEntity>& entities);

public:
    friend class Singleton<SchemeReader>;

    /**
     * Reads and caches the schema definitions from all JSON files in the specified directory.
     *
     * @param directoryPath The path to the directory containing schema JSON files.
     * @return A list of TableScheme objects representing the parsed schemas.
     */
    std::list<std::shared_ptr<EntityScheme>> readSchemes(const std::string& directoryPath);

    /**
     * Reads and parses a single schema definition from a JSON file.
     *
     * @param filePath The path to the JSON file containing a schema definition.
     * @return A TableScheme object representing the parsed schema.
     */
    std::shared_ptr<EntityScheme> readScheme(const fs::path& filePath);

    /**
     * Reads and aggregates entities from all JSON files in the specified directory.
     *
     * @param directoryPath The path to the directory containing entity JSON files.
     * @return An unordered map where keys are entity names and values are lists of JsonEntity objects.
     */
    std::unordered_map<std::string, std::list<JsonEntity>> readAllEntities(const std::string& directoryPath);

    /**
     * Reads and parses entities from a single JSON file.
     *
     * @param filePath The path to the JSON file containing entity definitions.
     * @return A list of JsonEntity objects representing the parsed entities.
     */
    std::list<JsonEntity> readEntities(const fs::path& filePath);

    /**
     * Retrieves the default directory path used for reading JSON schema and entity files.
     *
     * @return The default directory path for JSON files.
     */
    std::string getDefaultJsonsDirectoryPath() const;
};


#endif //SPHINXD_ENTITIESREADER_H
