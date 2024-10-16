#ifndef SPHINXD_JSONENTITYFILTER_H
#define SPHINXD_JSONENTITYFILTER_H

#include    <json/json.h>

/**
 * @class JsonEntityFilter
 *
 * @brief Provides functionality to filter JSON entities based on specified criteria.
 *
 * This class allows for filtering of JSON entities using a defined filter structure.
 * The filter criteria are specified in JSON format and can include various conditions
 * to match against JSON entities.
 */
class JsonEntityFilter {
    Json::Value filter;

public:
    /**
     * Constructor that initializes the filter with the specified JSON filter criteria.
     *
     * @param filterCriteria The JSON object defining filter criteria.
     */
    JsonEntityFilter(const Json::Value& filterCriteria);

    /**
     * Checks if the given JSON entity matches the filter criteria.
     *
     * @param data The JSON entity to be checked against the filter.
     * @return true if the entity matches the filter, false otherwise.
     */
    bool isFiltered(const Json::Value& data);

    // Constants
    static const std::string FILTERS;
    static const std::string SORTS; // для обработки ORDER BY
    static const std::string TYPE;
    static const std::string VALUE;
    static const std::string VALUES;
    static const std::string FIELD_NAME;
    static const std::string IS_ASCENDING; // сортировка по возрастанию если true

    // Operators
    static const std::string EQ;
    static const std::string NE;
    static const std::string LT;
    static const std::string LE;
    static const std::string GT;
    static const std::string GE;
    static const std::string IN_;
    static const std::string NIN; // not IN
    static const std::string AND;
    static const std::string OR;
    static const std::string IS;
    static const std::string NIS; // is not (not is)

private:
    /**
     * Checks if a single field in the entity matches the field filter.
     *
     * @param fieldFilter The filter criteria for a specific field.
     * @param dataField The data of the field to be checked.
     * @return true if the field matches the filter, false otherwise.
     */
    bool checkField(const Json::Value& fieldFilter, const Json::Value& dataField);

    /**
     * Checks if the entity matches a group of filters.
     *
     * @param groupFilter The filter criteria for a group of fields.
     * @param data The JSON entity to be checked.
     * @return true if the entity matches all filters in the group, false otherwise.
     */
    bool checkFilterGroup(const Json::Value& groupFilter, const Json::Value& data);
};

#endif //SPHINXD_JSONENTITYFILTER_H
