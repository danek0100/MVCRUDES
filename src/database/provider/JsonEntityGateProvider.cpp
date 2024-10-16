#include    "JsonEntityGateProvider.h"


JsonEntityGateProvider::JsonEntityGateProvider() : SimpleGateJsonEntityProvider<JsonEntity>(JsonEntityProvider::getSingletonInstance(), GATE_JSONENTITY_PROVIDER), Singleton<JsonEntityGateProvider>() {}

Json::Value JsonEntityGateProvider::dataToJson(const JsonEntity& data)
{
    return data.toJson();
}

JsonEntity JsonEntityGateProvider::jsonToData(const Json::Value& json)
{
    return {json};
}