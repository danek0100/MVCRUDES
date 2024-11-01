package com.cololo.tc.db.orm.jsonrepository.mapper;

import com.cololo.tc.db.orm.jsonrepository.exception.JsonEntityMappingException;
import com.cololo.tc.db.orm.provider.jsonEntity.JsonEntity;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;
import org.junit.Test;

import java.util.Objects;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThrows;

record ValidEntityRecord(
        int key,
        String name,
        Integer age,
        double height
) { }

class ValidEntityClass {
    public final int key;
    public final String name;
    public final Integer age;
    public final double height;

    public ValidEntityClass(int key, String name, Integer age, double height) {
        this.key = key;
        this.name = name;
        this.age = age;
        this.height = height;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ValidEntityClass that = (ValidEntityClass) o;
        return key == that.key && Double.compare(height, that.height) == 0 && Objects.equals(name, that.name) && Objects.equals(age, that.age);
    }

    @Override
    public int hashCode() {
        return Objects.hash(key, name, age, height);
    }
}

class InvalidConstructorEntity {
    public InvalidConstructorEntity(int x) { }
}

class NoKeyEntity {

}

public class JsonEntityMapperTest {
    private final Mapper<String, String> classNameMapper = new UpperCamelCaseMapper();

    @Test
    public void convertTo_record_success() throws JSONException {
        Mapper<JsonEntity, ValidEntityRecord> entityMapper = new JsonEntityMapper<>(ValidEntityRecord.class);
        ValidEntityRecord testEntity = new ValidEntityRecord(1, "Test", 35, 185.5);

        String className  = testEntity.getClass().getSimpleName();
        String entityName = classNameMapper.mapFrom(className).toUpperCase();
        JSONObject obj = new JSONObject()
                .put("NAME", testEntity.name())
                .put("AGE", testEntity.age())
                .put("HEIGHT", testEntity.height());

        JsonEntity exp = new JsonEntity(entityName, testEntity.key(), obj);

        JsonEntity act = entityMapper.mapTo(testEntity);

        assertEquals(exp, act);
    }

    @Test
    public void convertTo_class_success() throws JSONException {
        Mapper<JsonEntity, ValidEntityClass> entityMapper = new JsonEntityMapper<>(ValidEntityClass.class);
        ValidEntityClass testEntity = new ValidEntityClass(1, "Test", 35, 185.5);

        String className  = testEntity.getClass().getSimpleName();
        String entityName = classNameMapper.mapFrom(className).toUpperCase();
        JSONObject obj = new JSONObject()
                .put("NAME", testEntity.name)
                .put("AGE", testEntity.age)
                .put("HEIGHT", testEntity.height);

        JsonEntity exp = new JsonEntity(entityName, testEntity.key, obj);

        JsonEntity act = entityMapper.mapTo(testEntity);

        assertEquals(exp, act);
    }

    @Test
    public void convertFrom_record_success() throws JSONException {
        Mapper<JsonEntity, ValidEntityRecord> entityMapper = new JsonEntityMapper<>(ValidEntityRecord.class);
        ValidEntityRecord testEntity = new ValidEntityRecord(1, "Test", 35, 185.5);

        String className  = testEntity.getClass().getSimpleName();
        String entityName = classNameMapper.mapFrom(className);
        JSONObject obj = new JSONObject()
                .put("NAME", testEntity.name())
                .put("AGE", testEntity.age())
                .put("HEIGHT", testEntity.height());
        JsonEntity src = new JsonEntity(entityName, testEntity.key(), obj);

        ValidEntityRecord exp = new ValidEntityRecord(testEntity.key(), testEntity.name(), testEntity.age(), testEntity.height());

        ValidEntityRecord act = entityMapper.mapFrom(src);

        assertEquals(exp, act);
    }

    @Test
    public void convertFrom_class_success() throws JSONException {
        Mapper<JsonEntity, ValidEntityClass> entityMapper = new JsonEntityMapper<>(ValidEntityClass.class);
        ValidEntityClass testEntity = new ValidEntityClass(1, "Test", 35, 185.5);

        String className  = testEntity.getClass().getSimpleName();
        String entityName = classNameMapper.mapFrom(className);
        JSONObject obj = new JSONObject()
                .put("NAME", testEntity.name)
                .put("AGE", testEntity.age)
                .put("HEIGHT", testEntity.height);
        JsonEntity src = new JsonEntity(entityName, testEntity.key, obj);

        ValidEntityClass exp = new ValidEntityClass(testEntity.key, testEntity.name, testEntity.age, testEntity.height);

        ValidEntityClass act = entityMapper.mapFrom(src);

        assertEquals(exp, act);
    }

    @Test
    public void convertTo_noKey_exception() {
        Mapper<JsonEntity, NoKeyEntity> entityMapper = new JsonEntityMapper<>(NoKeyEntity.class);
        NoKeyEntity noKeyEntity = new NoKeyEntity();
        assertThrows(JsonEntityMappingException.class, () -> entityMapper.mapTo(noKeyEntity));
    }

    @Test
    public void constructor_invalidConstructor_exception() {
        Mapper<JsonEntity, InvalidConstructorEntity> entityMapper = new JsonEntityMapper<>(InvalidConstructorEntity.class);
        JsonEntity src = new JsonEntity("", 1, new JSONObject());
        assertThrows(JsonEntityMappingException.class, () -> entityMapper.mapFrom(src));
    }

    @Test
    public void convertFrom_invalidFields_exception() throws JSONException {
        Mapper<JsonEntity, ValidEntityClass> entityMapper = new JsonEntityMapper<>(ValidEntityClass.class);
        ValidEntityClass testEntity = new ValidEntityClass(1, "Test", 35, 185.5);

        String className  = testEntity.getClass().getSimpleName();
        String entityName = classNameMapper.mapFrom(className);
        JSONObject obj = new JSONObject()
                .put("FIRST_NAME", testEntity.name)
                .put("AGE", testEntity.age)
                .put("HEIGHT", testEntity.height);
        JsonEntity src = new JsonEntity(entityName, testEntity.key, obj);

        assertThrows(JsonEntityMappingException.class, () -> entityMapper.mapFrom(src));
    }
}
