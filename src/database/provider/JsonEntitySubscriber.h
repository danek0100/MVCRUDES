#ifndef SPHINXD_JSONENTITYSUBSCRIBER_H
#define SPHINXD_JSONENTITYSUBSCRIBER_H

#include    "JsonEntity.h"

class JsonEntitySubscriber {
public:
    std::string owner;
    std::function<void(uint32_t, JsonEntity)> callback;
    std::function<bool(JsonEntity)> filter;
    JsonEntitySubscriber(const std::string& _owner, const std::function<void(uint32_t, JsonEntity)>& _callback, const std::function<bool(JsonEntity)> _filter);

    bool operator==( const JsonEntitySubscriber &x ) const;
    bool operator!=( const JsonEntitySubscriber &x ) const;
};

#endif //SPHINXD_JSONENTITYSUBSCRIBER_H
