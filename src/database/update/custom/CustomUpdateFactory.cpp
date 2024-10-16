#include    "CustomUpdateFactory.h"

CustomUpdate* CustomUpdateFactory::createUpdate(const std::string& updateName) {
    auto it = getRegistry().find(updateName);
    return it == getRegistry().end() ? nullptr : (it->second)();
}

bool CustomUpdateFactory::registerUpdate(const std::string& updateName, std::function<CustomUpdate*()> constructor) {
    getRegistry()[updateName] = constructor;
    return true;
}

std::map<std::string, std::function<CustomUpdate*()>>& CustomUpdateFactory::getRegistry() {
    static std::map<std::string, std::function<CustomUpdate*()>> registry;
    return registry;
}