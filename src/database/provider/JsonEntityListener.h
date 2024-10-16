#ifndef SPHINXD_JSONENTITYLISTENER_H
#define SPHINXD_JSONENTITYLISTENER_H

#include    "JsonEntitySubscriber.h"
#include    "JsonEntity.h"
#include    "JsonEntityProvider.h"
#include    <utils/ObjectsSubscriber.h>
#include    <utils/Singleton.h>

/**
 * @class JsonEntityListener
 * @brief Listens to events related to JsonEntity and notifies subscribers.
 *
 * Inherits Singleton to ensure a single instance. Manages a list of subscribers and dispatches events
 * related to JsonEntity objects to these subscribers.
 */
class JsonEntityListener: public Singleton<JsonEntityListener> {
private:
    JsonEntityListener();
    Mutex mutexSubscribers;
    std::list<JsonEntitySubscriber> subscribers;

public:
    friend Singleton;

    static uint32_t EVENT_ADD;
    static uint32_t EVENT_UPDATE;
    static uint32_t EVENT_REMOVE;

    /**
     * Gets the name of the event corresponding to the provided event ID.
     *
     * @param event Event ID for which the name is required.
     * @return String representing the name of the event.
     */
    static std::string getEventName(uint32_t event);

    /**
     * Adds a subscriber to the list.
     *
     * @param subscriber The JsonEntitySubscriber to add.
     */
    void addSubscriber(const JsonEntitySubscriber& subscriber);

    /**
     * Removes a subscriber from the list.
     *
     * @param subscriber The JsonEntitySubscriber to remove.
     */
    void removeSubscriber(const JsonEntitySubscriber& subscriber);

    /**
     * Checks if a given subscriber is already in the subscribers list.
     *
     * @param subscriber The JsonEntitySubscriber to check.
     * @return True if the subscriber is present, False otherwise.
     */
    bool isSubscriber(const JsonEntitySubscriber& subscriber);

protected:
    /**
     * Notifies all subscribers about an event.
     *
     * Iterates through the list of subscribers and invokes their callback method if their filter matches.
     *
     * @param event The event ID to notify about.
     * @param data The JsonEntity data associated with the event.
     */
    void notifySubscribers(uint32_t event, const JsonEntity& data);
};

#endif //SPHINXD_JSONENTITYLISTENER_H
