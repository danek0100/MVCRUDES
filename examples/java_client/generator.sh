#!/bin/bash

check_dependencies() {
    if ! command -v jq >/dev/null 2>&1; then
        printf "jq is not installed. Attempting installation...\n" >&2
        local os_type=$(uname -s)
        case "$os_type" in
            Linux)
                if command -v apt-get >/dev/null; then
                    sudo apt-get update && sudo apt-get install -y jq || return 1
                elif command -v yum >/dev/null; then
                    sudo yum update && sudo yum install -y jq || return 1
                else
                    printf "Unsupported Linux package manager. Cannot automatically install jq.\n" >&2
                    return 1
                fi
                ;;
            Darwin)
                if command -v brew >/dev/null; then
                    brew install jq || return 1
                else
                    printf "Homebrew not found. Cannot automatically install jq on macOS.\n" >&2
                    return 1
                fi
                ;;
            CYGWIN*|MINGW32*|MSYS*|MINGW*)
                if ! curl -L -o /usr/bin/jq.exe https://github.com/stedolan/jq/releases/latest/download/jq-win64.exe; then
                    printf "Failed to download jq for Windows. Please install manually.\n" >&2
                    return 1
                fi
                ;;
            *)
                printf "Unsupported OS. Cannot automatically install jq. Please install manually.\n" >&2
                return 1
                ;;
        esac
        printf "jq successfully installed.\n"
    fi
}

# Convert UPPER_SNAKE_CASE to CamelCase
snake_to_camel() {
    local snake_str="$1"
    echo "${snake_str}" | awk -F'_' '{for(i=1;i<=NF;i++)$i=toupper(substr($i,1,1)) tolower(substr($i,2));}1' OFS=""
}

snake_to_java_field_name() {
    echo "$1" | awk 'BEGIN{FS="_";OFS=""}{for(i=1;i<=NF;i++){printf (i>1?toupper(substr($i,1,1)):tolower(substr($i,1,1))) tolower(substr($i,2))}}'
}


# Generate Java class from JSON string
generate_java_class_from_json() {
    local json_str="$1"
    local class_name
    local entity_name
    entity_name=$(echo "$json_str" | jq -r '.Name')
    class_name=$(snake_to_camel "$(echo "$json_str" | jq -r '.Name')")

    local package="package com.cololo.tc.generated.dto;\n\n"
    local imports="import com.cololo.tc.db.orm.provider.jsonEntity.JsonDto;\nimport com.cololo.tc.tools.json.JSONException;\nimport com.cololo.tc.tools.json.JSONObject;\nimport lombok.*;\n\n"
    local class_annotations="@AllArgsConstructor\n@NoArgsConstructor\n@Getter\n@Setter\n@EqualsAndHashCode\n@Builder\n"
    local class_declaration="public class ${class_name} implements JsonDto {\n\n"
    local fields="    private Long id;\n"
    local json_fields=""

    while IFS= read -r line; do
        local field_type camel_name field_name default_value cleaned_default_value
        IFS=' ' read -r type field_name default_value <<< "$line"
        # Clean the default_value by removing newlines and trimming spaces
        cleaned_default_value=$(echo "$default_value" | tr -d '\n' | xargs)
        camel_name=$(snake_to_java_field_name "$field_name")
        case "$type" in
            String) field_type="String";;
            Number) field_type="Integer";;
            *) printf "Unsupported type: %s\n" "$type" >&2; continue;;
        esac
        printf "cleaned_default_value: '%s'\n" "$cleaned_default_value" >&2;
        if [[ "$cleaned_default_value" != "null" && "$field_type" == "String" ]]; then
            fields+="    private ${field_type} ${camel_name} = \"${cleaned_default_value//\"/\\\"}\";\n"
            printf "default_value type: %s\n" "$field_type" >&2;
        elif [[ "$cleaned_default_value" == "null" ]]; then
            fields+="    private ${field_type} ${camel_name} = null;\n"
            printf "default_value_null type: %s\n" "$field_type" >&2;
        else
            fields+="    private ${field_type} ${camel_name} = ${cleaned_default_value};\n"
            printf "default_value_int type: %s\n" "$field_type" >&2;
        fi
        json_fields+="        object.put(\"${field_name}\", this.${camel_name});\n"
    done < <(echo "$json_str" | jq -r '.Fields[] | "\(.Type) \(.Name) \(.DefaultValue // "null")"')


    local methods=$(cat <<EOF

    @Override
    public JSONObject toJson() throws JSONException {
        JSONObject object = new JSONObject();
        object.put("entityName", getEntityName());
        object.put("ID", id);
$json_fields
        return object;
    }

    @Override
    public String getEntityName() {
        return "${entity_name}";
    }
EOF
)
    echo -e "${package}${imports}${class_annotations}${class_declaration}${fields}${methods}\n}\n"
}

# Add generation comment to Java code
add_generation_comment() {
    local java_code="$1"
    local timestamp
    timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    local comment="/*\n * This class was auto-generated on ${timestamp}\n * by a Bash script.\n */\n"
    echo -e "${comment}${java_code}"
}

# Clear directory contents
clear_directory() {
    local directory_path="$1"
    if [[ ! -d "${directory_path}" ]]; then
        mkdir -p "${directory_path}"
    else
        find "${directory_path}" -mindepth 1 -delete
    fi
}

# Generate and save Java classes from JSON files in a directory
generate_java_classes_from_directory() {
    local source_directory="$1"
    local target_directory="$2"
    clear_directory "${target_directory}"
    find "${source_directory}" -type f -name "*.json" | while read -r file_path; do
        local json_str
        json_str=$(<"${file_path}")
        local java_code
        if ! java_code=$(generate_java_class_from_json "${json_str}"); then
            printf "Error generating Java class from JSON.\n" >&2
            return 1
        fi
        local java_code_with_comment
        java_code_with_comment=$(add_generation_comment "${java_code}")
        local class_name
        class_name=$(snake_to_camel "$(echo "${json_str}" | jq -r '.Name')")
        local output_file_path="${target_directory}/${class_name}.java"
        echo -e "${java_code_with_comment}" > "${output_file_path}"
        printf "Generated %s\n" "${output_file_path}"
    done
}

generate_switch_cases() {
    local source_directory="$1"
    local switch_cases=""
    for json_file in "${source_directory}"/*.json; do
        local filename=$(basename -- "$json_file")
        local entity_name_uppercase=$(echo "${filename%.json}" | tr '[:lower:]' '[:upper:]')
        local class_name=$(snake_to_camel "${filename%.json}")
        switch_cases+="            case \"${entity_name_uppercase}\" -> { return ${class_name}.class; }"$'\n'
    done
    echo "$switch_cases"
}

# Generate DtoMapper.java
generate_dto_mapper_class() {
    local source_directory="$1"
    local output_directory="$2"
    local switch_cases=$(generate_switch_cases "$source_directory")

    local mapper_class_content="package com.cololo.tc.generated.dto;

import com.cololo.tc.db.orm.provider.jsonEntity.JsonDto;
import com.cololo.tc.db.orm.provider.jsonEntity.JsonMapper;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class DtoMapper {
    public static JsonDto mapJsonToDto(String jsonStr) throws Exception {
        ObjectMapper mapper = JsonMapper.getMapper();
        JsonNode rootNode = mapper.readTree(jsonStr);
        String entityName = rootNode.get(\"entityName\").asText();

        Class<? extends JsonDto> dtoClass = getDtoClass(entityName);
        if (dtoClass == null) {
            throw new IllegalArgumentException(\"Unknown entity: \" + entityName);
        }

        return mapper.treeToValue(rootNode, dtoClass);
    }

    private static Class<? extends JsonDto> getDtoClass(String entityName) {
        switch (entityName) {
$switch_cases
            default -> throw new IllegalArgumentException(\"Unknown entity: \" + entityName);
        }
    }
}"

    java_code_with_comment=$(add_generation_comment "${mapper_class_content}")
    echo "$java_code_with_comment" > "${output_directory}/DtoMapper.java"
    printf "Generated %s\n" "${output_directory}/DtoMapper.java"
}


main() {
    local source_directory="$1"
    local target_directory="$2"
    if ! check_dependencies; then
        printf "Failed to check or install dependencies. Script cannot proceed.\n" >&2
        return 1
    fi
    if [[ -z "$source_directory" || -z "$target_directory" ]]; then
        printf "Usage: %s <source_directory> <target_directory>\n" "$0" >&2
        return 1
    fi
    generate_java_classes_from_directory "$source_directory" "$target_directory"
    generate_dto_mapper_class "$source_directory" "$target_directory"
}

main "$@"
