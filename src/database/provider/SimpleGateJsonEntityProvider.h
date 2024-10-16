#ifndef SPHINXD_SIMPLEGATEJSONENTITYPROVIDER_H
#define SPHINXD_SIMPLEGATEJSONENTITYPROVIDER_H

#include    "JsonProviderWithFilteredSpeaker.h"
#include    "JsonEntityFilter.h"
#include    "JsonEntity.h"
#include    "./database/JsonsConstans.h"
#include    "./database/cache/EntitiesStructureCache.h"
#include    <gate/GateHandler.h>
#include    <gate/Message.h>
#include    <json/json.h>
#include    <utils/FastStreamWriter.h>


/**
 * @class SimpleGateJsonEntityProvider
 *
 * @brief Provides an interface for handling JSON entity interactions via a specific gate.
 *
 * This template class extends ObjectsSpeaker to facilitate the interaction of JSON entities
 * with a communication gate. It provides a standard interface for processing incoming requests,
 * handling subscriptions, and performing CRUD (Create, Read, Update, Delete) operations on
 * entities. The class also implements mechanisms for converting data to and from JSON format.
 *
 * @tparam Data The data type of the entities to be managed.
 */
template <typename Data>
class SimpleGateJsonEntityProvider : public ObjectsSpeaker<Json::Value> {
private:
    /**
     * Combines multiple filters into single filter group using given type
     *
     * @param filters Filters
     * @param type Type (AND / OR)
     * @return Filter group
     */
    static Json::Value combineFilters(const std::vector<Json::Value> &filters, const std::string &type);

public:
    // Constants representing different request and response types.
    const static int SUBSCRIBE = 0;
    const static int REMOVE_SUBSCRIBE = 1;
    const static int CREATE = 2;
    const static int READ = 3;
    const static int UPDATE = 4;
    const static int REMOVE = 5;
    const static int CUSTOM_COMMAND = 6;

    // Codes
    const static int OK = 0;
    const static int ERR = 1;

    /**
     * Constructs a SimpleGateJsonEntityProvider with the given provider and gate code.
     *
     * @param _provider Pointer to the JsonProviderWithFilteredSpeaker managing the data.
     * @param gateCode The code representing the gate for communication.
     */
    SimpleGateJsonEntityProvider(JsonProviderWithFilteredSpeaker<Data> *_provider, int gateCode);

    /**
     * Processes incoming requests from the gate.
     *
     * @param gate Reference to the GateHandler managing gate communication.
     * @param incomingMessage The received message to be processed.
     * @param trId The transaction ID of the incoming request.
     */
    void onIncomingRequest(GateHandler &gate, Message &incomingMessage, int trId);

    /**
     * Removes a gate subscriber from the provider.
     *
     * @param gate Reference to the GateHandler of the subscriber to be removed.
     */
    void removeGateSubscriber(GateHandler & gate);


    /**
     * Converts data to JSON format. Must be implemented in derived classes.
     *
     * @param data Data object to convert to JSON.
     * @return The JSON representation of the data.
     */
    virtual Json::Value dataToJson(const Data& data) = 0;

    /**
     * Converts JSON to data format. Must be implemented in derived classes.
     *
     * @param json JSON object to convert to data.
     * @return The data representation of the JSON.
     */
    virtual Data jsonToData(const Json::Value& json) = 0;
protected:
    int gateCode;
    JsonProviderWithFilteredSpeaker<Data> *provider;
};

template<typename Data>
Json::Value SimpleGateJsonEntityProvider<Data>::combineFilters(const std::vector<Json::Value> &filters, const std::string &type) {
    Json::Value groupFilter;
    groupFilter[JsonEntityFilter::TYPE] = type;

    Json::Value filterGroup(Json::ValueType::arrayValue);
    for (const Json::Value &filter : filters) {
        if (!filter.isNull()) {
            filterGroup.append(filter);
        }
    }

    groupFilter[JsonEntityFilter::FILTERS] = filterGroup;

    STATICTRACE(__FUNCTION__ << "() group filter: " << Json::FastWriter().write(groupFilter))
    return groupFilter;
}

template<typename Data>
SimpleGateJsonEntityProvider<Data>::SimpleGateJsonEntityProvider(JsonProviderWithFilteredSpeaker<Data> *_provider, int gateCode) :
        ObjectsSpeaker<Json::Value>(),
        gateCode(gateCode),provider(_provider) {

    provider->addSubscriber(ObjectsSubscriber<Data>(
            (std::uintptr_t)(this),
            std::function<void(uint32_t, Data)>([this](uint32_t event, Data data){notifySubscribers(event, dataToJson(data));}),
            std::function<bool(Data)>([](Data data){return true;})
    ));

    auto callback = [this](GateHandler &gate, int trId, Message &incomingMessage) {onIncomingRequest(gate,incomingMessage, trId);};
    auto removeCallback = [this](GateHandler &gate) {removeGateSubscriber(gate);};

    GateHandler::addRequestSubscriber(gateCode, callback);
    GateHandler::addDestructorSubscriber(removeCallback);
}

template<typename Data>
void SimpleGateJsonEntityProvider<Data>::onIncomingRequest(GateHandler &gate, Message &incomingMessage, int trId) {
    TRACE(__FUNCTION__ << "(), entry point. trId:[" << trId << "]");

    Message message;
    message.putReplyHeader(trId, gateCode);

    std::shared_ptr<Profile> profile = gate.getProfile();
    if (!profile) {
        TRACE(__FUNCTION__ << "() no profile")
        message.putInt(ERR);
        gate.sendMessage(&message);
        return;
    }

    try {
        int code = incomingMessage.extractInt();
        TRACE(__FUNCTION__ << "(), code:[" << code << "]");

        message.putInt(code);

        if (code == SUBSCRIBE) {
            Json::Reader jsonReader;
            Json::Value jsonData(Json::Value::null);
            std::string filter = incomingMessage.extractStr();
            TRACE(__FUNCTION__  << "() filter:[" << filter << "]");
            if(filter.empty() || jsonReader.parse(filter, jsonData)) {
                std::function<void(uint32_t, Json::Value)> callback = [&gate, this](uint32_t event, const Json::Value& data) {
                    TRACE(__FUNCTION__ << "(), callback entry")
                    Json::Value jsonEvent;
                    jsonEvent["event"] = getEventName(event);
                    jsonEvent["object"] = data;

                    Json::FastStreamWriter writer;
                    std::ostringstream os;
                    writer.write(os, jsonEvent);

                    Message message;
                    message.putAsyncHeader(gateCode);
                    message.putStr(os.str().c_str());

                    TRACE(__FUNCTION__ << "(), callback end")
                    gate.sendMessage(&message);
                };

                std::function<bool(Json::Value)> filterFunc = [profile, jsonData, this](const Json::Value& data) {
                    // Блокируем ивенты по сущностям, где не нужен провайдер.
                    if (data[ENTITY_NAME_FIELD].isString() && !EntitiesStructureCache::getSingletonInstance()->isNeedProvider(data[ENTITY_NAME_FIELD].asString())) {
                        return false;
                    }

                    Json::Value profileReadFilter = profile->getReadFilter(data[ENTITY_NAME_FIELD].asString());
                    Json::Value comboFilter = combineFilters({ jsonData, profileReadFilter }, JsonEntityFilter::AND);
                    TRACE(__FUNCTION__ << Json::FastWriter().write(comboFilter))

                    JsonEntityFilter jsonFilter(comboFilter);
                    return jsonFilter.isFiltered(data);
                };

                ObjectsSubscriber<Json::Value> subscriber(((std::uintptr_t)(&gate)), callback, filterFunc);

                addSubscriber(subscriber);
                message.putInt(OK);
            } else {
                message.putInt(ERR);
            }
        } else if (code == REMOVE_SUBSCRIBE) {
            ObjectsSubscriber<Json::Value> subscriber = ObjectsSubscriber<Json::Value>(((std::uintptr_t)(&gate)), nullptr,nullptr);

            removeSubscriber(subscriber);
            message.putInt(OK);
        } else if (code == CREATE) {
            Json::Reader jsonReader;
            Json::Value jsonData;
            if (jsonReader.parse(incomingMessage.extractStr(), jsonData) && jsonData.isArray()) {
                std::list<JsonEntity> toCreate;
                for (unsigned i = 0; i < jsonData.size(); ++i) {
                    toCreate.emplace_back(jsonData[i]);
                }
                provider->create(toCreate);
                message.putInt(OK);

                Json::Value createdArray = Json::Value(Json::arrayValue);
                for (const auto& object : toCreate) {
                    createdArray.append(dataToJson(object));
                }

                Json::FastStreamWriter writer;
                std::ostringstream os;
                writer.write(os, createdArray);
                message.putLongStr(os.str().c_str());
            } else {
                message.putInt(ERR);
            }
        } else if (code == READ) {
            std::string type = incomingMessage.extractStr();

            Json::Reader jsonReader;
            Json::Value json(Json::Value::null);
            std::string filter = incomingMessage.extractStr();
            TRACE(__FUNCTION__  << "() filter:[" << filter << "]");
            if(filter.empty() || jsonReader.parse(filter, json)) {
                std::list<Data> result;

                Json::Value profileReadFilter = gate.getProfile()->getReadFilter(type);
                Json::Value comboFilter = combineFilters({ json, profileReadFilter }, JsonEntityFilter::AND);
                TRACE(__FUNCTION__ << Json::FastWriter().write(comboFilter))

                Json::Value fieldsJson(Json::Value::null);
                std::string fieldsStr = incomingMessage.extractStr();
                if(fieldsStr.empty() || !jsonReader.parse(fieldsStr, fieldsJson) || !fieldsJson.isArray()) {
                    result = provider->read(type, comboFilter, {});
                } else {
                    std::set<std::string> fields {KEY_FIELD};
                    for (unsigned i = 0; i < fieldsJson.size(); ++i) {
                        if (fieldsJson[i].isString()) {
                            fields.insert(fieldsJson[i].asString());
                        }
                    }
                    result = provider->read(type, comboFilter, fields);
                }
                message.putInt(OK);

                Json::Value resultJson(Json::arrayValue);
                for (const auto &item: result) {
                    resultJson.append(dataToJson(item));
                }

                Json::FastStreamWriter writer;
                std::ostringstream os;
                writer.write(os, resultJson);
                message.putLongStr(os.str().c_str());
            } else {
                message.putInt(ERR);
            }
        } else if (code == UPDATE) {
            Json::Reader jsonReader;
            Json::Value jsonData;
            if (jsonReader.parse(incomingMessage.extractStr(), jsonData) && jsonData.isArray()) {
                std::list<JsonEntity> toUpdate;
                for (unsigned i = 0; i < jsonData.size(); ++i) {
                    const Json::Value &entity = jsonData[i];
                    if (profile->canUpdateEntity(entity)) {
                        toUpdate.emplace_back(entity);
                    }
                }
                provider->update(toUpdate);
                message.putInt(OK);
            } else {
                message.putInt(ERR);
            }
        } else if (code == REMOVE) {
            Json::Reader jsonReader;
            Json::Value jsonData;
            if(jsonReader.parse(incomingMessage.extractStr(), jsonData) && jsonData.isArray()) {
                std::list<JsonEntity> toRemove;
                for (unsigned i = 0; i < jsonData.size(); ++i) {
                    const Json::Value &entity = jsonData[i];
                    if (profile->canRemoveEntity(entity)) {
                        toRemove.emplace_back(entity);
                    }
                }
                provider->remove(toRemove);
                message.putInt(OK);
            } else {
                message.putInt(ERR);
            }
        }
        else {
            message.putInt(ERR);
            TRACE(__FUNCTION__ << "(), return error state, code:[" << code << "]");
        }
    } catch (EndOfBufferException* pxE) {
        TRACE( __FUNCTION__ << "() Got corrupted message" );
        delete pxE;
        message.putInt(ERR);
    } catch (SqlException* pxE) {
        TRACE( __FUNCTION__ << "() cannot to do sql operation" );
        delete pxE;
        message.putInt(ERR);
    }

    gate.sendMessage(&message);

    TRACE(__FUNCTION__ << "(), end point");
}

template<typename Data>
void SimpleGateJsonEntityProvider<Data>::removeGateSubscriber(GateHandler & gate) {
    TRACE(__FUNCTION__ << "() entry point. gate:[" << (std::uintptr_t)(&gate) <<"]")
    removeSubscriber(ObjectsSubscriber<Json::Value>(((std::uintptr_t)(&gate)), nullptr, nullptr));
    TRACE(__FUNCTION__ << "() end point")
}

#endif //SPHINXD_SIMPLEGATEJSONENTITYPROVIDER_H
