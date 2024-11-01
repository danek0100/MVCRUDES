package com.cololo.tc.db.orm.entities;

import com.cololo.tc.server.Message;
import com.cololo.tc.server.ServerRequestor;
import com.cololo.tc.tools.json.JSONArray;
import com.cololo.tc.tools.json.JSONException;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.mockito.*;
import com.cololo.tc.tools.json.JSONObject;

import java.util.Set;

import static com.cololo.tc.db.orm.entities.JsonEntitiesStructuresHolder.ENTITY_FIELDS;
import static com.cololo.tc.db.orm.entities.JsonEntitiesStructuresHolder.FIELD_NAME;
import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.mockStatic;

public class JsonEntitiesStructuresHolderTest {

    @BeforeEach
    public void setup() {
        // Очистка JsonEntitiesStructuresHolder перед каждым тестом
        JsonEntitiesStructuresHolder.getInstance().clear();
    }

    @Test
    public void loadDataFromServerTest() throws Exception {
        // Подготовка данных, которые должен вернуть мок ServerRequestor
        String fakeServerResponse = "{\"Entities\":[{\"Name\":\"TestEntity\",\"Fields\":[{\"Name\":\"field1\",\"Type\":\"Number\"}]}]}";
        Message fakeMessage = new Message();
        fakeMessage.putStr(fakeServerResponse);
        fakeMessage.setPosition(4);

        // Мокирование статического метода
        try (MockedStatic<ServerRequestor> mockedRequestor = mockStatic(ServerRequestor.class)) {
            mockedRequestor.when(() -> ServerRequestor.request(any(Message.class), anyInt())).thenReturn(fakeMessage);

            // Вызов тестируемого метода
            JsonEntitiesStructuresHolder.getInstance().loadDataFromServer();

            // Проверка, что данные были загружены корректно
            JSONObject structure = JsonEntitiesStructuresHolder.getInstance().getEntityStructure("TestEntity");
            assertNotNull(structure);
            assertEquals("TestEntity", structure.getString(FIELD_NAME));
            assertTrue(structure.getJSONArray(ENTITY_FIELDS).length() > 0);
        }
        JsonEntitiesStructuresHolder.getInstance().clear();
    }

    @Test
    public void testInit() throws JSONException {
        // Создаем тестовую структуру JSON
        JSONObject structures = new JSONObject();
        structures.put("Entities", new JSONArray().put(new JSONObject().put(FIELD_NAME, "TestEntity")));

        // Вызов метода init
        JsonEntitiesStructuresHolder.getInstance().init(structures);

        // Проверяем, что структуры были инициализированы
        assertFalse(JsonEntitiesStructuresHolder.getInstance().isNeedInit());
        assertNotNull(JsonEntitiesStructuresHolder.getInstance().getEntityStructure("TestEntity"));
    }

    @Test
    public void testGetAvailableEntities() throws JSONException {
        // Инициализируем структуры
        JSONObject structures = new JSONObject();
        structures.put("Entities", new JSONArray().put(new JSONObject().put(FIELD_NAME, "TestEntity")));
        JsonEntitiesStructuresHolder.getInstance().init(structures);

        // Вызов метода getAvailableEntities
        Set<String> entities = JsonEntitiesStructuresHolder.getInstance().getAvailableEntities();

        // Проверяем, что доступные сущности содержат "TestEntity"
        assertTrue(entities.contains("TestEntity"));
    }

    @Test
    public void testGetEntityStructure() throws JSONException {
        // Инициализируем структуры
        JSONObject structures = new JSONObject();
        structures.put("Entities", new JSONArray().put(new JSONObject().put(FIELD_NAME, "TestEntity")));
        JsonEntitiesStructuresHolder.getInstance().init(structures);

        // Вызов метода getEntityStructure с именем сущности
        JSONObject structure = JsonEntitiesStructuresHolder.getInstance().getEntityStructure("TestEntity");

        // Проверяем, что структура возвращается корректно
        assertNotNull(structure);
        assertEquals("TestEntity", structure.getString(FIELD_NAME));
    }

    @Test
    public void testIsNeedInit() throws JSONException {
        // Проверяем значение needInit перед инициализацией
        assertTrue(JsonEntitiesStructuresHolder.getInstance().isNeedInit());

        // Инициализация структур
        JSONObject structures = new JSONObject();
        structures.put("Entities", new JSONArray().put(new JSONObject().put(FIELD_NAME, "TestEntity")));
        JsonEntitiesStructuresHolder.getInstance().init(structures);

        // Проверяем значение needInit после инициализации
        assertFalse(JsonEntitiesStructuresHolder.getInstance().isNeedInit());
    }

    @Test
    public void testClear() throws JSONException {
        // Инициализируем структуры
        JSONObject structures = new JSONObject();
        structures.put("Entities", new JSONArray().put(new JSONObject().put(FIELD_NAME, "TestEntity")));
        JsonEntitiesStructuresHolder.getInstance().init(structures);

        // Убедимся, что структуры были инициализированы
        assertFalse(JsonEntitiesStructuresHolder.getInstance().isNeedInit());

        // Вызов метода clear
        JsonEntitiesStructuresHolder.getInstance().clear();

        // Проверяем, что структуры были очищены
        assertTrue(JsonEntitiesStructuresHolder.getInstance().isNeedInit());
        assertTrue(JsonEntitiesStructuresHolder.getInstance().getAvailableEntities().isEmpty());
    }

}
