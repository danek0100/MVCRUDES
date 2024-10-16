#ifndef SPHINXD_ENUMTYPESCHEME_H
#define SPHINXD_ENUMTYPESCHEME_H

#include    "EntityScheme.h"
#include    <string>
#include    <utility>
#include    <vector>

/**
 * @class EnumTypeScheme
 *
 * @brief Represents the schema of a database enum type within the application.
 *
 * It encapsulates the enum type's metadata, including its name and possible values.
 */
class EnumTypeScheme : public EntityScheme {
public:
    const std::vector<std::string> values;

    EnumTypeScheme( std::string typeName,
                    int typeVersion,
                    bool requiresProvider,
                    std::vector<std::string> enumValues)
           : EntityScheme(std::move(typeName), typeVersion, requiresProvider), values(std::move(enumValues)) {}
};

#endif //SPHINXD_ENUMTYPESCHEME_H
