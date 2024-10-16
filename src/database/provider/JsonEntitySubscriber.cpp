#include "JsonEntitySubscriber.h"


JsonEntitySubscriber::JsonEntitySubscriber(const std::string& _owner, const std::function<void(uint32_t, JsonEntity)>& _callback, const std::function<bool(JsonEntity)> _filter) {
    this->owner = _owner;
    this->callback = _callback;
    this->filter = _filter;
}


bool JsonEntitySubscriber::operator==( const JsonEntitySubscriber &x ) const {
    return this->owner == x.owner;
}

bool JsonEntitySubscriber::operator!=( const JsonEntitySubscriber &x ) const {
    return this->owner != x.owner;
}
