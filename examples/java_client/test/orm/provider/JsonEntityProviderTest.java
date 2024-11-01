package com.cololo.tc.db.orm.provider;

import com.cololo.tc.db.orm.provider.jsonEntity.JsonEntity;
import com.cololo.tc.db.orm.provider.jsonEntity.JsonEntityProvider;
import com.cololo.tc.server.Server;
import com.cololo.tc.tools.json.JSONArray;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.mockito.Mock;

import static com.cololo.tc.db.orm.provider.jsonEntity.JsonEntity.ENTITY_NAME_FIELD;
import static com.cololo.tc.db.orm.provider.jsonEntity.JsonEntity.KEY_FIELD;
import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.*;

import org.mockito.MockedStatic;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;

public class JsonEntityProviderTest {

    @Mock
    private Server serverMock;
    private MockedStatic<Server> mockedServer;

    @BeforeEach
    public void setUp() {
        // Инициализируем моки
        MockitoAnnotations.initMocks(this);

        // Конфигурация мока Server для возвращения поддельного ответа
        when(serverMock.isConnected()).thenReturn(true);
        mockedServer = mockStatic(Server.class);
        mockedServer.when(Server::get).thenReturn(serverMock);
    }

    @Test
    public void getInstanceTest() {
        assertNotEquals(JsonEntityProvider.getInstance(), null);
    }

    @Test
    public void testJsonToData() throws JSONException {
        JSONObject json = new JSONObject();
        json.put(ENTITY_NAME_FIELD, "testEntity");
        json.put(KEY_FIELD, 1);

        JsonEntityProvider provider = JsonEntityProvider.getInstance();
        JsonEntity result = provider.jsonToData(json);

        assertNotNull(result);
        assertEquals("testEntity", result.name);
        assertEquals(1, result.key);
    }

    @Test
    public void dataToJsonTest() throws JSONException {
        JsonEntity jsonEntity = new JsonEntity("JsonEntity", 1, new JSONObject().put("VALUE", "VAL"));
        JSONObject object = JsonEntityProvider.getInstance().dataToJson(jsonEntity);

        assertEquals(object.getString(ENTITY_NAME_FIELD), "JsonEntity");
        assertEquals(object.getInt(KEY_FIELD), 1);
        assertEquals(object.getString("VALUE"), "VAL");
    }

    @Test
    public void testCreate() throws Exception {
        // Создаем фиктивный объект JsonEntity для тестирования
        JSONArray jsonArray = new JSONArray();
        JSONObject jsonObject = new JSONObject();
        jsonObject.put(ENTITY_NAME_FIELD, "testEntity");
        jsonObject.put(KEY_FIELD, 1);
        jsonObject.put("VALUE", "VAL");
        jsonArray.put(jsonObject);

        JsonEntity toCreate = new JsonEntity("testEntity", -1, new JSONObject().put("VALUE", "VAL"));

        // Подготавливаем мок Server для возвращения JSON представления testEntity
        when(serverMock.gateProviderEntitiesCreateRequest(anyInt(), any(JSONArray.class))).thenReturn(jsonArray);

        // Вызов метода create и проверка результата
        JsonEntity result = JsonEntityProvider.getInstance().create(toCreate);
        assertNotNull(result);
        assertEquals("testEntity", result.name);
        assertEquals(1, result.key);
    }

    @Test
    public void testRemove() throws Exception {
        JsonEntity toRemove = new JsonEntity("testEntity", 2, new JSONObject().put("VALUE", "VAL"));
        when(serverMock.gateProviderRemoveRequest(anyInt(), any(JSONObject.class))).thenReturn(true);
        JsonEntityProvider.getInstance().remove(toRemove);
    }

    @Test
    public void testUpdate() throws Exception {
        JsonEntity toUpdate = new JsonEntity("testEntity", 2, new JSONObject().put("VALUE", "VAL"));
        when(serverMock.gateProviderUpdateRequest(anyInt(), any(JSONObject.class))).thenReturn(true);
        JsonEntityProvider.getInstance().remove(toUpdate);
    }

    @Test
    public void testRead() throws Exception {
        // Создаем JSONArray, который будет возвращен сервером
        JSONArray jsonArray = new JSONArray();
        JSONObject jsonEntity1 = new JSONObject().put(ENTITY_NAME_FIELD, "testEntity1").put(KEY_FIELD, 1).put("VALUE", "VAL1");
        JSONObject jsonEntity2 = new JSONObject().put(ENTITY_NAME_FIELD, "testEntity2").put(KEY_FIELD, 2).put("VALUE", "VAL2");
        jsonArray.put(jsonEntity1).put(jsonEntity2);

        // Конфигурация мока Server
        when(serverMock.gateProviderEntitiesReadRequest(anyInt(), anyString(), any(JsonFilter.class), any(JSONArray.class))).thenReturn(jsonArray);

        // Фильтр для чтения сущностей
        JsonFilter filter = new JsonFilter((JSONObject)null);

        // Вызов метода read и проверка результата
        JsonEntityProvider provider = JsonEntityProvider.getInstance();
        ArrayList<JsonEntity> result = provider.read("entityType", filter, new JSONArray());

        assertNotNull(result);
        assertEquals(2, result.size()); // Проверяем, что получили 2 сущности
        assertEquals("testEntity1", result.get(0).name);
        assertEquals(1, result.get(0).key);
        assertEquals("VAL1", result.get(0).value.getString("VALUE"));
        assertEquals("testEntity2", result.get(1).name);
        assertEquals(2, result.get(1).key);
        assertEquals("VAL2", result.get(1).value.getString("VALUE"));
    }

    @Test
    public void testListenerManagement() {
        FilteredCallback<JsonEntity> listener = mock(FilteredCallback.class);
        JsonEntityProvider.getInstance().addProviderListener(listener);

        // Проверяем, что слушатель добавлен
        List<FilteredCallback<JsonEntity>> listeners = JsonEntityProvider.getInstance().getProviderListeners();
        assertTrue(listeners.contains(listener));

        // Удаляем слушателя и проверяем, что он удален
        JsonEntityProvider.getInstance().removeProviderListener(listener);
        assertFalse(listeners.contains(listener));
    }

    @Test
    public void testNotifyProviderListeners() throws JSONException {
        JsonEntity entity = new JsonEntity("testEntity", 2, new JSONObject().put("VALUE", "VAL"));
        JsonFilter filter = new JsonFilter((JSONObject)null);

        FilteredCallback<JsonEntity> listener = mock(FilteredCallback.class, withSettings().useConstructor(filter));
        JsonEntityProvider.getInstance().addProviderListener(listener);

        JsonEntityProvider.getInstance().notifyProviderListeners(Event.ADD, entity);

        // Проверяем, что метод call был вызван у слушателя
        verify(listener).call(Event.ADD, entity);
    }

    @AfterEach
    public void tearDown() {
        mockedServer.close();
    }
}
