#ifndef SPHINXD_ITYPESCONVERTER_H
#define SPHINXD_ITYPESCONVERTER_H

#include    "./database/provider/JsonEntity.h"
#include    "./sql/BaseDbClientProvider.h"
#include    <string>
#include    <vector>
#include    <unordered_map>
#include    <list>

class ITypesConverter : public BaseDbClientProvider {
public:

    /**
     * Virtual destructor for safe polymorphic deletion.
     */
    virtual ~ITypesConverter() = default;

    /**
     * Adapts a value to be compatible with a specific type in the database.
     *
     * This method is used for converting values into a format suitable for database insertion or comparison.
     *
     * @param typeName The type with which the value needs to be compatible.
     * @param value The value to be adapted.
     * @return The adapted value as a string.
     */
    virtual std::string adaptValueToType(const std::string& typeName, const Json::Value& value) = 0;

    /**
     * Gets the database-specific mapped type for a generic type name.
     *
     * @param typeName The generic type name.
     * @param length The length for types that require it, such as VARCHAR.
     * @return The database-specific type name mapped from the generic type.
     */
    virtual std::string getMappedType(const std::string& typeName, int length) = 0;

    /**
     * Gets the generic type name for a database-specific type string.
     *
     * @param typeString The database-specific type string.
     * @return The generic type name unmapped from the database-specific type.
     */
    virtual std::string getUnmappedType(const std::string& typeString) = 0;
};

#endif //SPHINXD_ITYPESCONVERTER_H
