#include    "JsonEntityListener.h"

JsonEntityListener::JsonEntityListener() : Singleton<JsonEntityListener>(), mutexSubscribers("speaker subscribers mutex") {
    std::function<void(uint32_t, JsonEntity)> iCallback = [this](uint32_t event, const JsonEntity& data) {
        this->notifySubscribers(event, data);
    };

    std::function<bool(JsonEntity)> iFilterFunc = [](const JsonEntity& data) {
        return true;
    };

    ObjectsSubscriber<JsonEntity> entitiesSubscriber(((std::uintptr_t)(this)), iCallback, iFilterFunc);
    JsonEntityProvider::getSingletonInstance()->addSubscriber(entitiesSubscriber);
}

uint32_t JsonEntityListener::EVENT_ADD = 0;
uint32_t JsonEntityListener::EVENT_UPDATE = 1;
uint32_t JsonEntityListener::EVENT_REMOVE = 2;
std::string JsonEntityListener::getEventName(uint32_t event) {
    if (event == EVENT_ADD) return "ADD";
    if (event == EVENT_UPDATE) return "UPDATE";
    if (event == EVENT_REMOVE) return "REMOVE";

    return "unknown";
}

void JsonEntityListener::addSubscriber(const JsonEntitySubscriber& subscriber) {
    TRACE( __FUNCTION__  << "(). [" << subscriber.owner << "] entry point")
    MutexLock lock(&mutexSubscribers);
    if (std::find(subscribers.cbegin(), subscribers.cend(), subscriber) != subscribers.cend()) {
        TRACE(__FUNCTION__ << "(): object is subscriber already.");
    } else {
        subscribers.push_back(subscriber);
    }
    TRACE( __FUNCTION__  << "(). end point")
}

void JsonEntityListener::removeSubscriber(const JsonEntitySubscriber& subscriber) {
    TRACE( __FUNCTION__ << "() entry point")
    MutexLock lock(&mutexSubscribers);
    subscribers.remove(subscriber);
    TRACE( __FUNCTION__ << "() end point")
}

void JsonEntityListener::notifySubscribers(uint32_t event, const JsonEntity& data) {
    TRACE( __FUNCTION__ << "() entry point")
    MutexLock lock(&mutexSubscribers);
    for (const auto &item: subscribers) {
        if (item.filter(data)) item.callback(event, data);
    }
    TRACE( __FUNCTION__ << "() end point")
}

bool JsonEntityListener::isSubscriber(const JsonEntitySubscriber& subscriber) {
    TRACE( __FUNCTION__ << "() entry point")
    MutexLock lock(&mutexSubscribers);
    return std::find(subscribers.begin(), subscribers.end(), subscriber) != subscribers.end();
}
