package com.cololo.tc.db.orm.provider.jsonEntity;

import com.cololo.tc.db.orm.entities.JsonEntitiesStructuresHolder;
import com.cololo.tc.tools.json.JSONArray;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;

import java.util.Objects;

import static com.cololo.tc.db.orm.entities.JsonEntitiesStructuresHolder.*;

/**
 * Represents an entity with a name and a corresponding JSON value.
 * This class is immutable, ensuring that its state cannot be changed after creation.
 */
public final class JsonEntity {
    public final String name;
    public final int key;
    public final JSONObject value; //TODO научить библиотеку определять тип значения isInt и т.д.

    public static final String ENTITY_NAME_FIELD = "entityName";
    public static final String KEY_FIELD = "ID";

    /**
     * Constructs a new JsonEntity with the specified name and JSON value.
     *
     * @param name  The type name of entity, not null.
     * @param key  The unique key of the entity, not null.
     * @param value The JSON value of the entity, not null.
     * @throws IllegalArgumentException if any argument is null.
     */
    public JsonEntity(String name, int key, JSONObject value) {
        if (name == null || value == null) {
            throw new IllegalArgumentException("Name and value must not be null");
        }
        this.name = name;
        this.key = key;
        this.value = value;
    }

    public JsonEntity(JSONObject value) {
        try
        {
            this.name = value.getString(ENTITY_NAME_FIELD);
            this.key = value.has(KEY_FIELD) ? value.getInt(KEY_FIELD) : -1;
            this.value = value;
        }
        catch (JSONException e)
        {
            throw new IllegalArgumentException("Name and value must not be null");
        };
    }

    /**
     * Returns a string representation of the JsonEntity.
     *
     * @return A string that includes the entity's name, key and value.
     */
    @Override
    public String toString() {
        return "JsonEntity{entityName='" + name + "', key=" + key + ", value=" + value + "}";
    }

    public String getName() {
        return name;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof JsonEntity)) return false;

        JsonEntity jsonEntity = (JsonEntity) o;

        // Прямое сравнение полей
        if (key != jsonEntity.key) return false;
        if (!name.equals(jsonEntity.name)) return false;

        // Получаем структуру и проверяем наличие полей перед сравнением
        JSONObject structure = JsonEntitiesStructuresHolder.getInstance().getEntityStructure(name);
        if (structure != null) {
            try {
                JSONArray fields = structure.getJSONArray(ENTITY_FIELDS);

                for (int i = 0; i < fields.length(); ++i) {
                    JSONObject field = fields.getJSONObject(i);
                    String fieldName = field.getString(FIELD_NAME);
                    String fieldType = field.getString(FIELD_TYPE);

                    // Проверяем наличие поля перед сравнением
                    if (fieldType.equals(JSON_INT_TYPE) &&
                            (value.isNull(fieldName) || jsonEntity.value.isNull(fieldName) ||
                                    value.getInt(fieldName) != jsonEntity.value.getInt(fieldName))) {
                        return false;
                    } else if (fieldType.equals(JSON_STRING_TYPE) &&
                            (value.isNull(fieldName) || jsonEntity.value.isNull(fieldName) ||
                                    !value.getString(fieldName).equals(jsonEntity.value.getString(fieldName)))) {
                        return false;
                    }
                }
            } catch (Exception e) {
                return false;
            }
        } else {
            // Простое сравнение JSON объектов, если структура не определена
            return Objects.equals(value.toString(), jsonEntity.value.toString());
        }
        return true;
    }

    @Override
    public int hashCode() {
        int result = Objects.hash(name, key);

        JSONObject structure = JsonEntitiesStructuresHolder.getInstance().getEntityStructure(name);
        if (structure != null) {
            try {
                JSONArray fields = structure.getJSONArray(ENTITY_FIELDS);

                for (int i = 0; i < fields.length(); ++i) {
                    JSONObject field = fields.getJSONObject(i);
                    String fieldName = field.getString(FIELD_NAME);
                    String fieldType = field.getString(FIELD_TYPE);

                    if (!value.isNull(fieldName)) {
                        if (fieldType.equals(JSON_INT_TYPE)) {
                            result = 31 * result + Integer.hashCode(value.getInt(fieldName));
                        } else if (fieldType.equals(JSON_STRING_TYPE)) {
                            result = 31 * result + value.getString(fieldName).hashCode();
                        }
                    }
                }
            } catch (Exception e) {
                result = 31 * result;
            }
        } else {
            // Если структура не определена, используем хеш-код всего JSON объекта
            result = 31 * result + value.hashCode();
        }

        return result;
    }
}
