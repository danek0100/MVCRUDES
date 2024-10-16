#ifndef SPHINXD_IENTITIESSTRUCTURECACHE_H
#define SPHINXD_IENTITIESSTRUCTURECACHE_H

#include <string>

class IEntitiesStructureCache {
public:
    /**
     * Virtual destructor for safe polymorphic deletion.
     */
    virtual ~IEntitiesStructureCache() = default;

    /**
     * Retrieves the possible values of an enum type.
     *
     * @param key The identifier of the enum entity.
     * @return A vector of strings representing enum value names.
     */
    virtual std::vector<std::string> getEnumValues(const std::string& key) = 0;

    /**
     * Calculates whether type is enum and there are predefined values.
     *
     * @param key The identifier of the enum entity.
     * @return true, if type is enum, false otherwise.
     */
    virtual bool isTypeEnum(const std::string& key) = 0;
};

#endif //SPHINXD_IENTITIESSTRUCTURECACHE_H
