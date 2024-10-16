#ifndef SPHINXD_JSONPROVIDER_H
#define SPHINXD_JSONPROVIDER_H

#include    <json/value.h>
#include    <list>

/**
 * @class JsonProvider
 * @brief Abstract base class template for providing CRUD operations on JSON objects.
 *
 * This template class defines a generic interface for CRUD (Create, Read, Update, Delete)
 * operations on objects of type Object, which are expected to be represented or convertible
 * to/from JSON format.
 *
 * @tparam Object The type of objects that this provider will handle. It is expected that
 *        this type can be converted to and from Json::Value.
 */
template<typename Object>
class JsonProvider {
public:
    /**
     * Creates new objects in the system and fill absent values in place.
     *
     * @param objects The objects to be created.
     */
    virtual void create(std::list<Object>& objects) = 0;

    /**
     * Reads a list of objects based on the specified filter criteria.
     *
     * @param filter A Json::Value representing the filter criteria.
     * @return A list of objects matching the filter criteria.
     */
    virtual std::list<Object> read(const std::string& type, const Json::Value& filter) = 0;

    /**
     * Reads a list of objects based on the specified filter criteria.
     *
     * @param filter A Json::Value representing the filter criteria.
     * @param fields Vector with fields for entities.
     * @return A list of objects matching the filter criteria.
     */
    virtual std::list<Object> read(const std::string& type, const Json::Value& filter, const std::set<std::string>& fields) = 0;

    /**
     * Updates existing objects in the system.
     *
     * @param objects The objects to update, typically identified by an ID.
     */
    virtual void update(std::list<Object>& objects) = 0;

    /**
     * Removes objects from the system.
     *
     * @param objects The objects to be removed, typically identified by an ID.
     */
    virtual void remove(std::list<Object>& objects) = 0;
};

#endif //SPHINXD_JSONPROVIDER_H
