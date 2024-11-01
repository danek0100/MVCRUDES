package com.cololo.tc.db.orm.entities;

import com.cololo.tc.Common;
import com.cololo.tc.MainWindow;
import com.cololo.tc.server.*;
import com.cololo.tc.tools.json.JSONArray;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;

import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;
import java.util.Set;

/**
 * Singleton class to hold and manage JSON entities structures.
 */
public class JsonEntitiesStructuresHolder {
    private static final JsonEntitiesStructuresHolder instance = new JsonEntitiesStructuresHolder();
    private final Map<String, JSONObject> entities = new ConcurrentHashMap<>();
    private volatile boolean needInit = true;

    public static final String ENTITY_FIELDS = "Fields";
    public static final String FIELD_NAME = "Name";
    public static final String FIELD_TYPE = "Type";
    public static final String JSON_INT_TYPE = "Number";
    public static final String JSON_STRING_TYPE = "String";

    private JsonEntitiesStructuresHolder() {}

    /**
     * @return the single instance of JsonEntitiesStructuresHolder
     */
    public static JsonEntitiesStructuresHolder getInstance() {
        return instance;
    }

    /**
     * Updates JSON structures from the server. Clears existing entities and reloads them.
     */
    public void updateStructuresFromServer() {
        try {
            entities.clear();
            loadDataFromServer();
            needInit = false;
        } catch (Exception e) {
            Common.error(MainWindow.get(), Common.getStr("JsonEntitiesStructuresHolder.initError"));
        }
    }

    /**
     * Initializes the holder with a given JSONObject containing structures.
     *
     * @param structures JSONObject containing the structures
     */
    public void init(JSONObject structures) {
        try {
            entities.clear();
            parseEntities(structures);
            needInit = false;
        } catch (Exception e) {
            Common.error(MainWindow.get(), Common.getStr("JsonEntitiesStructuresHolder.initError"));
        }
    }

    /**
     * Initializes the holder with a JSONObject loaded from server after login.
     */
    protected void loadDataFromServer() throws Exception {
        Message request = createEntityRequest();
        Message reply = ServerRequestor.request(request, 3000);
        parseEntities(new JSONObject(reply.extractStr()));
    }

    private Message createEntityRequest() throws Message.EndOfBufferException {
        Message request = new Message();
        request.putRequestHeader(ServerProtocol.GATE_JSONENTITIES_REQUEST);
        return request;
    }

    private void parseEntities(JSONObject jsonObject) throws JSONException {
        JSONArray entitiesArray = jsonObject.getJSONArray("Entities");
        for (int i = 0; i < entitiesArray.length(); ++i) {
            parseEntity(entitiesArray.getJSONObject(i));
        }
    }

    private void parseEntity(JSONObject entity) throws JSONException {
        String tableName = entity.getString(FIELD_NAME);
        entities.put(tableName, entity);
    }

    /**
     * Retrieves the set of names of available entities. If not initialized, updates structures from server.
     *
     * @return a Set of entity names
     */
    public Set<String> getAvailableEntities() {
        return entities.keySet();
    }

    /**
     * Retrieves the structure of a specified entity. If not initialized, updates structures from server.
     *
     * @param entityName the name of the entity
     * @return JSONObject representing the structure of the entity
     */
    public JSONObject getEntityStructure(String entityName) {
        return entities.get(entityName);
    }

    /**
     * Return is need init from Json or Server.
     *
     * @return needInit boolean value
     */
    public boolean isNeedInit() {
        return needInit;
    }

    /**
     * Drop entities structures.
     */
    public void clear() {
        entities.clear();
        needInit = true;
    }
}
