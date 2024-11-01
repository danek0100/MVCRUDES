package com.cololo.tc.db.orm.jsonrepository.mapper;

import com.cololo.tc.Common;
import com.cololo.tc.db.orm.jsonrepository.exception.JsonEntityMappingException;
import com.cololo.tc.db.orm.provider.jsonEntity.JsonEntity;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.*;

/**
 * Конвертер сущностей из Java-объектов в JsonEntity.
 * @restrictions
 * 1) Класс сущности должен предоставлять публичный конструктор с параметрами для инициализации всех полей.
 * 2) Порядок параметров в конструкторе должен соответствовать порядку полей в классе.
 * 3) Тип параметров в конструкторе должен соответствовать типам соответствующих полей в классе.
 * 4) Класс должен иметь поле "key" типа int.
 * 5) Класс должен иметь название в UpperCamelCase и соответствовать SNAKE_CASE названию JsonEntity.
 * 6) Поля класса должны иметь названия в lowerCamelCase и соответствовать SNAKE_CASE названиям полей JsonEntity.
 * 7) Унаследованные поля класса конвертером игнорируются.
 * @param <T> Тип сущности (Java).
 */
public class JsonEntityMapper<T> implements Mapper<JsonEntity, T> {
    private static final String KEY_FIELD_NAME = "key";
    private final Mapper<String, String> fieldNameConverter = new LowerCamelCaseMapper();
    private final Mapper<String, String> classNameConverter = new UpperCamelCaseMapper();
    private final Class<T> clazz;
    private final Constructor<T> constructor;

    /**
     * @param clazz Класс сущности.
     * @throws JsonEntityMappingException Если не удалось получить конструктор.
     */
    public JsonEntityMapper(Class<T> clazz) {
        this.clazz = clazz;

        Constructor<T> declaredConstructor = null;
        try {
            List<? extends Class<?>> fieldTypesList = Arrays.stream(clazz.getDeclaredFields()).map(Field::getType).toList();
            Class<?>[] fieldTypesArray = fieldTypesList.toArray(new Class<?>[0]);
            declaredConstructor = clazz.getDeclaredConstructor(fieldTypesArray);
        } catch (NoSuchMethodException e) {
            Common.logException(e);
        }

        this.constructor = declaredConstructor;
    }

    /**
     * @param source Java-объект.
     * @throws JsonEntityMappingException Если не удалось сопоставить значения полей Java-сущности полям JsonEntity.
     * @return JsonEntity.
     */
    @Override
    public JsonEntity mapTo(T source) {
        // Конвертируем имя класса в SNAKE_CASE
        String className  = clazz.getSimpleName();
        String entityName = classNameConverter.mapFrom(className).toUpperCase();

        JSONObject jsonObject = new JSONObject();
        int key;

        try {
            // Получаем поле "key"
            Field keyField = clazz.getDeclaredField(KEY_FIELD_NAME);
            keyField.setAccessible(true);
            key = keyField.getInt(source);

            // Проходимся по полям класса
            for (Field field : clazz.getDeclaredFields()) {
                if (field.getName().equals(KEY_FIELD_NAME)) continue;
                field.setAccessible(true);

                // Имя каждого поля конвертируем в SNAKE_CASE, кладем значение по нему в jsonObject
                String fieldName = field.getName();
                String jsonFieldName = fieldNameConverter.mapFrom(fieldName).toUpperCase();
                Object value = field.get(source);

                jsonObject.put(jsonFieldName, value);
            }

        } catch (NoSuchFieldException e) {
            throw new JsonEntityMappingException("Error mapping fields: " + e.getMessage());
        } catch (IllegalAccessException e) {
            throw new JsonEntityMappingException("Error accessing field: " + e.getMessage());
        } catch (JSONException e) {
            throw new JsonEntityMappingException("Error creating json: " + e.getMessage());
        }

        return new JsonEntity(entityName, key, jsonObject);
    }

    /**
     * @param source JsonEntity.
     * @throws JsonEntityMappingException Если не удалось сопоставить JsonEntity Java-сущности.
     * @return Java-объект.
     */
    @Override
    public T mapFrom(JsonEntity source) {
        if (constructor == null) {
            throw new JsonEntityMappingException("The constructor with the required parameters is not defined");
        }

        String className = clazz.getSimpleName();

        String jsonEntityName = source.name;
        String entityName = classNameConverter.mapTo(jsonEntityName);

        // Если название сущности в UpperCamelCase не совпадает с названием класса
        if (!entityName.equals(className)) {
            throw new JsonEntityMappingException("Entity name doesn't match class name");
        }

        try {
            JSONObject jsonObject = source.value;

            Field[] fields = clazz.getDeclaredFields();
            Object[] values = new Object[fields.length];

            // Считываем значения всех полей JsonEntity, соответствующих полям Java-сущности
            for (int i = 0; i < fields.length; i++) {
                if (fields[i].getName().equals(KEY_FIELD_NAME)) {
                    values[i] = source.key;
                } else {
                    String fieldName = fields[i].getName();
                    String jsonFieldName = fieldNameConverter.mapFrom(fieldName).toUpperCase();
                    values[i] = jsonObject.get(jsonFieldName);
                }
            }

            // Пытаемся создать Java-объект
            return constructor.newInstance(values);

        } catch (IllegalAccessException e) {
            throw new JsonEntityMappingException("Error accessing field: " + e.getMessage());
        } catch (InstantiationException | InvocationTargetException e) {
            throw new JsonEntityMappingException("Error creating instance: " + e.getMessage());
        } catch (JSONException e) {
            throw new JsonEntityMappingException("Error creating json: " + e.getMessage());
        }
    }
}
