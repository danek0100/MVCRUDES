#ifndef SPHINXD_JSONSCONSTANS_H
#define SPHINXD_JSONSCONSTANS_H

// JsonsEntitiesConsts
namespace {
    const std::string JSON_PATH_VARIABLE = "JSONS_PATH";

    const std::string INT_JSON_TYPE = "Number";
    const std::string FLOAT_JSON_TYPE = "Float";
    const std::string STRING_JSON_TYPE = "String";
    const std::string BOOLEAN_JSON_TYPE = "Boolean";
    const std::string BINARY_JSON_TYPE = "Binary";
    const std::string DATE_JSON_TYPE = "Date";
    const std::string DATETIME_JSON_TYPE = "Datetime";

    const std::string ENTITY_NAME_FIELD = "entityName";
    const std::string KEY_FIELD = "ID";

    const std::string FIELD_NAME = "Name";
    const std::string FIELD_TYPE = "Type";
    const std::string DEFAULT_VALUE_PROPERTY = "DefaultValue";
    const std::string CAN_BE_EMPTY_PROPERTY = "CanBeEmpty";
    const std::string LENGTH_PROPERTY = "Length";

    const std::string VERSIONS_TABLE_NAME = "TABLESVERSIONS";

    const std::string VERSION_PROPERTY = "Version";
    const std::string NEED_PROVIDER_PROPERTY = "NeedProvider";
    const std::string FIELDS_PROPERTY = "Fields";
    const std::string INDEXES_PROPERTY = "Indexes";
    const std::string UNIQUE_FIELDS_SETS_PROPERTY = "UniqueFieldsSets";
    const std::string RELATIONS_PROPERTY = "Relations";
    const std::string ENGINES_PROPERTY = "DatabaseEngines";
    const std::string DEFAULT_VALUES_PROPERTY = "DefaultValues";

    const std::string RELATIONS_LOCAL_FIELDS = "LocalFields";
    const std::string RELATIONS_TARGET = "Target";
    const std::string RELATIONS_TARGET_NAME = "EntityName";
    const std::string RELATIONS_TARGET_FIELDS = "TargetFields";
    const std::string RELATIONS_ON_DELETE = "OnDelete";
    const std::string RELATIONS_ON_UPDATE = "OnUpdate";

    static const int DEFAULT_LENGTH = 255;
}

#endif //SPHINXD_JSONSCONSTANS_H
