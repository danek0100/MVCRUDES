#ifndef SPHINXD_JSONENTITYGATEPROVIDER_H
#define SPHINXD_JSONENTITYGATEPROVIDER_H

#include    "SimpleGateJsonEntityProvider.h"
#include    "JsonEntity.h"
#include    "JsonEntityProvider.h"
#include    "./database/JsonsConstans.h"
#include    "./gate/gateprotocol.h"
#include    "./utils/Singleton.h"
#include    <utility>


/**
 * @class JsonEntityGateProvider
 *
 * @brief Provides gate-specific functionalities for JSON entities.
 *
 * This class, implemented as a singleton, extends SimpleGateEntityProvider to handle JSON entities.
 * It is designed to provide gate-specific functionalities, which involve converting JSON entities
 * to and from the JSON format. This is particularly useful for managing JSON entities across different
 * gate codes, where each gate represents a distinct set of functionalities or protocols.
 */
class JsonEntityGateProvider : public SimpleGateJsonEntityProvider<JsonEntity>,  public Singleton<JsonEntityGateProvider> {
private:
    JsonEntityGateProvider();
public:
    friend Singleton;

protected:
    /**
     * Converts a JsonEntity object to a Json::Value.
     * This method is used to serialize JsonEntity objects into a JSON format
     * for transmission or storage.
     *
     * @param data The JsonEntity object to be converted.
     * @return The Json::Value representation of the JsonEntity.
     */
    Json::Value dataToJson(const JsonEntity& data) override;

    /**
     * Converts a Json::Value to a JsonEntity object.
     * This method is used to deserialize JSON data into a JsonEntity object
     * for processing within the application.
     *
     * @param json The Json::Value to be converted.
     * @return The JsonEntity representation of the Json::Value.
     */
    JsonEntity jsonToData(const Json::Value& json) override;
};

#endif //SPHINXD_JSONENTITYGATEPROVIDER_H
