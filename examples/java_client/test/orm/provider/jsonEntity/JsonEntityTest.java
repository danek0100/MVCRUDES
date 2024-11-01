package com.cololo.tc.db.orm.provider.jsonEntity;

import com.cololo.tc.db.orm.entities.JsonEntitiesStructuresHolder;
import com.cololo.tc.tools.json.JSONArray;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static com.cololo.tc.db.orm.entities.JsonEntitiesStructuresHolder.*;
import static org.junit.jupiter.api.Assertions.*;


public class JsonEntityTest {

    @BeforeEach
    public void setup() throws JSONException {
        JSONObject testStructure = new JSONObject();
        JSONArray entitiesArray = new JSONArray();

        JSONObject entity = new JSONObject();
        entity.put(FIELD_NAME, "TestDeepEntity");
        JSONArray fields = new JSONArray();
        fields.put(new JSONObject().put(FIELD_NAME, "numField").put(FIELD_TYPE, JSON_INT_TYPE));
        fields.put(new JSONObject().put(FIELD_NAME, "strField").put(FIELD_TYPE, JSON_STRING_TYPE));
        entity.put(ENTITY_FIELDS, fields);

        entitiesArray.put(entity);
        testStructure.put("Entities", entitiesArray);

        JsonEntitiesStructuresHolder.getInstance().init(testStructure);
    }

    @Test
    public void testEntityCreation() throws JSONException {
        String name = "TestEntity";
        int key = 1;
        JSONObject value = new JSONObject();
        value.put("data", "testValue");

        JsonEntity jsonEntity = new JsonEntity(name, key, value);

        assertEquals(name, jsonEntity.getName());
        assertEquals(key, jsonEntity.key);
        assertEquals(value.toString(), jsonEntity.value.toString());
    }

    @Test
    public void testEntityCreationWithNullValues() {
        String name = null;
        int key = -1;
        JSONObject value = null;

        assertThrows(IllegalArgumentException.class, () -> new JsonEntity(name, key, value));
    }

    @Test
    public void testToString() throws JSONException {
        String name = "TestEntity";
        int key = 1;
        JSONObject value = new JSONObject();
        value.put("data", "testValue");

        JsonEntity jsonEntity = new JsonEntity(name, key, value);

        String expected = "JsonEntity{entityName='" + name + "', key=" + key + ", value=" + value + "}";
        assertEquals(expected, jsonEntity.toString());
    }

    @Test
    public void testGetName() throws JSONException {
        String name = "TestEntity";
        int key = 1;
        JSONObject value = new JSONObject();
        value.put("data", "testValue");

        JsonEntity jsonEntity = new JsonEntity(name, key, value);

        assertEquals(name, jsonEntity.getName());
    }

    @Test
    public void testEqualsForEqualObjects() throws JSONException {
        String name = "TestEntity";
        int key = 1;
        JSONObject value = new JSONObject();
        value.put("data", "testValue");

        JsonEntity jsonEntity1 = new JsonEntity(name, key, value);
        JsonEntity jsonEntity2 = new JsonEntity(name, key, value);

        assertEquals(jsonEntity1, jsonEntity2);
    }

    @Test
    public void testEqualsForNonEqualObjects() throws JSONException {
        String name1 = "TestEntity1";
        String name2 = "TestEntity2";
        int key = 1;
        JSONObject value = new JSONObject();
        value.put("data", "testValue");

        JsonEntity jsonEntity1 = new JsonEntity(name1, key, value);
        JsonEntity jsonEntity2 = new JsonEntity(name2, key, value);

        assertNotEquals(jsonEntity1, jsonEntity2);
    }

    @Test
    public void testHashCodeForEqualObjects() throws JSONException {
        String name = "TestEntity";
        int key = 1;
        JSONObject value = new JSONObject();
        value.put("data", "testValue");

        JsonEntity jsonEntity1 = new JsonEntity(name, key, value);
        JsonEntity jsonEntity2 = new JsonEntity(name, key, value);

        assertEquals(jsonEntity1.hashCode(), jsonEntity2.hashCode());
    }

    @Test
    public void testHashCodeForChangedValues() throws JSONException {
        String name = "TestEntity";
        int key = 1;
        JSONObject value1 = new JSONObject();
        value1.put("data", "testValue");

        JSONObject value2 = new JSONObject();
        value2.put("data", "differentValue");

        JsonEntity jsonEntity1 = new JsonEntity(name, key, value1);
        JsonEntity jsonEntity2 = new JsonEntity(name, key, value2);

        assertNotEquals(jsonEntity1.hashCode(), jsonEntity2.hashCode());
    }

    @Test
    public void testDeepEquals() throws JSONException {
        JSONObject value1 = new JSONObject().put("numField", 10).put("strField", "test");
        JSONObject value2 = new JSONObject().put("numField", 10).put("strField", "test");

        JsonEntity entity1 = new JsonEntity("TestDeepEntity", 1, value1);
        JsonEntity entity2 = new JsonEntity("TestDeepEntity", 1, value2);

        assertEquals(entity1, entity2);
    }

    @Test
    public void testDeepHashCode() throws JSONException {
        JSONObject value1 = new JSONObject().put("numField", 10).put("strField", "test");
        JSONObject value2 = new JSONObject().put("numField", 10).put("strField", "test");

        JsonEntity entity1 = new JsonEntity("TestDeepEntity", 1, value1);
        JsonEntity entity2 = new JsonEntity("TestDeepEntity", 1, value2);

        assertEquals(entity1.hashCode(), entity2.hashCode());
    }

}
