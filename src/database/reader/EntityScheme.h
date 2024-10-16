#ifndef SPHINXD_ENTITYSCHEME_H
#define SPHINXD_ENTITYSCHEME_H

#include    <string>
#include    <utility>
#include    <vector>
#include    <unordered_map>

/**
 * @class EntityScheme
 *
 * @brief Represents the schema of a database type within the application.
 *
 * It encapsulates the type's metadata, including its name and version.
 * This class is used to define and manipulate the structure of enum types in a database-centric application,
 * ensuring that the application's data model is consistent with the underlying database structure.
 */
class EntityScheme {
public:
    const std::string name;
    const int version;
    const bool needProvider;

    EntityScheme( std::string typeName, int typeVersion, bool requiresProvider)
    : name(std::move(typeName)), version(typeVersion), needProvider(requiresProvider) {}

    virtual ~EntityScheme() {};
};

#endif //SPHINXD_ENTITYSCHEME_H
