package com.cololo.tc.db.orm.jsonrepository.mapper;

import java.util.regex.Pattern;

/**
 * Конвертер идентификаторов из snake_case (case-insensitive) в UpperCamelCase.
 */
public class UpperCamelCaseMapper implements Mapper<String, String> {
    /**
     * @param source snake_case string.
     * @return UpperCamelCase string
     */
    @Override
    public String mapTo(String source) {
        String result = Pattern.compile("_([a-z])([a-z]+)")
                .matcher(source.toLowerCase())
                .replaceAll(m -> m.group(1).toUpperCase().concat(m.group(2)))
                .replaceAll("_", "");
        return result.isEmpty() ? "" : result.substring(0, 1).toUpperCase().concat(result.substring(1));
    }

    /**
     * @param source UpperCamelCase string.
     * @return snake_case string (lower case)
     */
    @Override
    public String mapFrom(String source) {
        return Pattern.compile("([a-z]+)([A-Z])")
                .matcher(source)
                .replaceAll(m -> m.group(1).concat("_").concat(m.group(2)))
                .toLowerCase();
    }
}
