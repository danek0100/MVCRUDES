#include    "JsonEntityFilter.h"

const std::string JsonEntityFilter::FILTERS = "filters";
const std::string JsonEntityFilter::SORTS = "sorts";
const std::string JsonEntityFilter::TYPE = "type";
const std::string JsonEntityFilter::VALUE = "value";
const std::string JsonEntityFilter::VALUES = "values";
const std::string JsonEntityFilter::FIELD_NAME = "fieldName";
const std::string JsonEntityFilter::IS_ASCENDING = "isAscending";

const std::string JsonEntityFilter::EQ = "EQ";
const std::string JsonEntityFilter::NE = "NE";
const std::string JsonEntityFilter::LT = "LT";
const std::string JsonEntityFilter::LE = "LE";
const std::string JsonEntityFilter::GT = "GT";
const std::string JsonEntityFilter::GE = "GE";
const std::string JsonEntityFilter::IN_ = "IN";
const std::string JsonEntityFilter::NIN = "NIN";
const std::string JsonEntityFilter::AND = "AND";
const std::string JsonEntityFilter::OR = "OR";
const std::string JsonEntityFilter::IS = "IS";
const std::string JsonEntityFilter::NIS = "NIS";


JsonEntityFilter::JsonEntityFilter(const Json::Value& _filter) : filter(_filter) {}

bool JsonEntityFilter::isFiltered(const Json::Value& data) {
    if (filter.isNull()) return true;

    if (!filter[FILTERS].isNull()) return checkFilterGroup(filter, data);
    else return checkField(filter, data);
}

bool JsonEntityFilter::checkFilterGroup(const Json::Value& groupFilter, const Json::Value& data) {
    std::string type = groupFilter[TYPE].asString();

    if (type == AND) {
        for (const auto &item: groupFilter[FILTERS]) {
            if (!item[JsonEntityFilter::FILTERS].isNull() && item[FILTERS].isArray()) {
                if (!checkFilterGroup(item, data)) return false;
            } else {
                if (!checkField(item, data)) return false;
            }
        }
        return true;
    }
    else if (type == OR) {
        for (const auto &item: groupFilter[FILTERS]) {
            if (!item[FILTERS].isNull() && item[FILTERS].isArray()) {
                if (checkFilterGroup(item, data)) return true;
            } else {
                if (checkField(item, data)) return true;
            }
        }
        return false;
    }
    return false;
}

bool JsonEntityFilter::checkField(const Json::Value& fieldFilter, const Json::Value& data) {
    std::string fieldName = fieldFilter[FIELD_NAME].asString();
    std::string type = fieldFilter[TYPE].asString();

    if (type == EQ) {
        return data[fieldName] == fieldFilter[VALUE];
    } else if (type == NE) {
        return data[fieldName] != fieldFilter[VALUE];
    } else if (type == LT) {
        return data[fieldName] < fieldFilter[VALUE];
    } else if (type == LE) {
        return data[fieldName] <= fieldFilter[VALUE];
    } else if (type == GT) {
        return data[fieldName] > fieldFilter[VALUE];
    } else if (type == GE) {
        return data[fieldName] >= fieldFilter[VALUE];
    } else if (type == IN_) {
        for (const auto &item: fieldFilter[VALUES]) {
            if (data[fieldName] == item) return true;
        }
    } else if (type == NIN) {
        for (const auto &item: fieldFilter[VALUES]) {
            if (data[fieldName] == item) return false;
        }
        return true;
    }
    return false;
}
