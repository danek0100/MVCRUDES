#ifndef SPHINXD_JSONPROVIDERWITHFILTEREDSPEAKER_H
#define SPHINXD_JSONPROVIDERWITHFILTEREDSPEAKER_H

#include    "JsonProvider.h"
#include    "./utils/ObjectsSpeaker.h"
#include    "./utils/ObjectsSubscriber.h"
#include    <json/value.h>


/**
 * @class JsonProviderWithFilteredSpeaker
 * @brief Extends JsonProvider with filtered speaking capabilities for data objects.
 *
 * This template class combines the functionality of JsonProvider with ObjectsSpeaker
 * to provide CRUD (Create, Read, Update, Delete) operations along with the ability
 * to 'speak' or notify about these operations. It acts as an intermediary, handling
 * the basic CRUD operations and delegating the implementation specifics to derived
 * classes through protected implementation methods.
 *
 * The class is designed to be used where you need both data manipulation and
 * communication/notification capabilities tied to these data operations.
 *
 * @tparam Data The type of objects that this provider will handle. The type should
 *        be convertible to and from Json::Value and suitable for communication via
 *        ObjectsSpeaker.
 */
template <typename Data>
class JsonProviderWithFilteredSpeaker : private JsonProvider<Data>, public ObjectsSpeaker<Data> {
public:
    /**
     * Creates a new object and fill it in place. The actual creation logic is implemented in createImpl.
     *
     * @param objects The objects to be created.

     */
    void create(std::list<Data>& objects) override;

    /**
     * Reads a list of objects based on a filter. The actual reading logic is implemented in readImpl.
     *
     * @param filter The JSON filter criteria.
     * @return A list of objects matching the filter.
     */
    std::list<Data> read(const std::string& type, const Json::Value& filter) override;

    /**
     * Reads a list of objects based on a filter. The actual reading logic is implemented in readImpl.
     *
     * @param filter The JSON filter criteria.
     * @param fields The fields to entities.
     * @return A list of objects matching the filter.
     */
    std::list<Data> read(const std::string& type, const Json::Value& filter, const std::set<std::string>& fields) override;

    /**
     * Updates an existing objects. The actual update logic is implemented in updateImpl.
     *
     * @param objects The objects to be updated.
     */
    void update(std::list<Data>& objects) override;

    /**
     * Removes objects. The actual removal logic is implemented in removeImpl.
     *
     * @param objects The objects to be removed.
     */
    void remove(std::list<Data>& objects) override;

protected:
    /**
     * Implementation of object creation.
     *
     * @param object The object to be created and filled in place.
     */
    virtual void createImpl(std::list<Data>& object) = 0;

    /**
     * Implementation of reading objects based on a filter.
     *
     * @param filter The JSON filter criteria.
     * @return A list of objects matching the filter.
     */
    virtual std::list<Data> readImpl(const std::string& type, const Json::Value& filter, const std::set<std::string>& fields = {}, bool forceRead = false) = 0;

    /**
     * Implementation of updating objects.
     *
     * @param objects The objects to be updated.
     */
    virtual std::list<Data> updateImpl(std::list<Data>& objects) = 0;

    /**
     * Implementation of removing an object.
     *
     * @param object The object to be removed.
     */
    virtual std::list<Data> removeImpl(std::list<Data>& objects) = 0;
};

template <typename Data>
void JsonProviderWithFilteredSpeaker<Data>::create(std::list<Data>& objects) {
    TRACE(__FUNCTION__ << "() entry point")
    createImpl(objects);
    for (const auto& object : objects) {
        this->notifySubscribers(this->EVENT_ADD, object);
    }
    TRACE(__FUNCTION__ << "() end point")
}

template <typename Data>
std::list<Data> JsonProviderWithFilteredSpeaker<Data>::read(const std::string& type, const Json::Value& filter) {
    TRACE(__FUNCTION__ << "() entry point")
    return readImpl(type, filter, {});
}

template <typename Data>
std::list<Data> JsonProviderWithFilteredSpeaker<Data>::read(const std::string& type, const Json::Value& filter, const std::set<std::string>& fields) {
    TRACE(__FUNCTION__ << "() entry point")
    return readImpl(type, filter, fields);
}

template <typename Data>
void JsonProviderWithFilteredSpeaker<Data>::update(std::list<Data>& objects) {
    TRACE(__FUNCTION__ << "() entry point")
    std::list<Data> entities = updateImpl(objects);
    for (const auto& object : entities) {
        this->notifySubscribers(this->EVENT_UPDATE, object);
    }
    TRACE(__FUNCTION__ << "() end point")
}

template <typename Data>
void JsonProviderWithFilteredSpeaker<Data>::remove(std::list<Data>& objects) {
    TRACE(__FUNCTION__ << "() entry point")
    std::list<Data> result = removeImpl(objects);
    for (const auto& object : result) {
        this->notifySubscribers(this->EVENT_REMOVE, object);
    }
    TRACE(__FUNCTION__ << "() end point")
}

#endif //SPHINXD_JSONPROVIDERWITHFILTEREDSPEAKER_H
