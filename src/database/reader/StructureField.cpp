#include "StructureField.h"

StructureField::StructureField() {}

StructureField::StructureField(std::initializer_list<std::pair<std::string, Json::Value>> list) {
    for (const auto& pair : list) {
        features.insert(pair);
    }
}

Json::Value StructureField::get(const std::string& key) const {
    auto it = features.find(key);
    if (it != features.end()) {
        return it->second;
    }
    return {Json::ValueType::nullValue};
}

void StructureField::put(const std::string& key, const Json::Value& value) {
    features[key] = value;
}

std::vector<std::string> StructureField::getKeys() const {
    std::vector<std::string> keys;
    keys.reserve(features.size());  // Reserve space for efficiency.
    for (const auto& feature : features) {
        keys.push_back(feature.first);
    }
    return keys;
}

bool StructureField::operator==(const StructureField& other) const {
    // Сначала проверим количество ключей.
    if (features.size() != other.features.size()) {
        return false;
    }

    // Проверяем, совпадают ли все ключи и их значения.
    for (const auto& pair : features) {
        const std::string& key = pair.first;
        const Json::Value& value = pair.second;

        auto it = other.features.find(key);
        if (it == other.features.end() || it->second != value) {
            return false;
        }
    }

    return true;
}

bool StructureField::operator!=(const StructureField& other) const {
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &stream, const StructureField &field) {
    stream << "StructureField{";
    for(const auto &feature : field.features) {
        stream << "\"" << feature.first << "\":"
               << Json::FastWriter().write(feature.second);
        stream.seekp(-1, stream.cur);
        stream << ",";
    }
    stream.seekp(-1, stream.cur);
    return stream << "}";
}
