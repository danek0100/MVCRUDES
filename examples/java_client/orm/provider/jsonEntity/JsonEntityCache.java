package com.cololo.tc.db.orm.provider.jsonEntity;

import com.cololo.tc.Common;
import com.cololo.tc.db.orm.provider.Event;
import com.cololo.tc.db.orm.provider.FilteredCallback;
import com.cololo.tc.db.orm.provider.JsonFilter;
import com.cololo.tc.server.Message;
import com.cololo.tc.server.Server;
import com.cololo.tc.tools.editor.EntityNotifier;
import com.cololo.tc.tools.json.JSONArray;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutionException;

import static com.cololo.tc.db.orm.provider.jsonEntity.JsonEntity.ENTITY_NAME_FIELD;
import static com.cololo.tc.db.orm.provider.jsonEntity.JsonEntity.KEY_FIELD;
import static com.cololo.tc.server.ServerProtocol.GATE_JSONENTITY_PROVIDER;

public class JsonEntityCache implements EntityNotifier<JsonEntity>
{
    private static final List<String> ENTITIES = Arrays.asList("PARAMI", "PARAMB", "DEVICES_SECURITY_DATA","ZONES");
    private static final ConcurrentHashMap<String, ConcurrentHashMap<Integer, JsonEntity>> cache = new ConcurrentHashMap<>();
    private boolean needInit = true;

    private static final FilteredCallback<JsonEntity> listener = new FilteredCallback<>(createJsonFilter())
    {
        @Override
        public void call(Event event, JsonEntity jsonEntity)
        {
            ConcurrentHashMap<Integer, JsonEntity> cacheData = cache.get(jsonEntity.getName());

            if (cacheData == null) return;

            switch (event)
            {
                case ADD, UPDATE -> cacheData.put(jsonEntity.key, jsonEntity);
                case REMOVE -> cacheData.remove(jsonEntity.key);
            }
            instance.notifyEntityListeners(jsonEntity);
        }
    };

    private final static JsonEntityCache instance = new JsonEntityCache();

    private JsonEntityCache()
    {
        ENTITIES.forEach(name -> cache.computeIfAbsent(name, k -> new ConcurrentHashMap<>()));
       JsonEntityProvider.getInstance().addProviderListener(listener);
       Server.get().addConnectionStateListener(()-> {if(!Server.get().isConnected()) needInit = true;});
    }

    private static JsonFilter createJsonFilter()
    {
        try
        {
            JSONArray filtersArray = new JSONArray();
            for (String paramName : ENTITIES) {
                filtersArray.put(new JSONObject()
                        .put(JsonFilter.FIELD_NAME, ENTITY_NAME_FIELD)
                        .put(JsonFilter.TYPE, JsonFilter.EQ)
                        .put(JsonFilter.VALUE, paramName));
            }
            return new JsonFilter(new JSONObject()
                    .put(JsonFilter.FILTERS, filtersArray)
                    .put(JsonFilter.TYPE, JsonFilter.OR));
        }
        catch (Exception e)
        {
            Common.logException(e);
        }

        return null;
    }

    /**
     * Initializes the cache with entities.
     */
    private synchronized void initializeCache()
    {
        if (Server.get().isConnected() && needInit)
        {
            ENTITIES.forEach(name ->
            {
                cache.get(name).clear();
                try
                {
                    JSONArray array = Server.gateProviderEntitiesReadRequest(GATE_JSONENTITY_PROVIDER, name, null, null);
                    if (array == null) return;
                    for (int i = 0; i < array.length(); i++) {
                        String entityName = (String) array.getJSONObject(i).remove(ENTITY_NAME_FIELD);
                        int key = (Integer) array.getJSONObject(i).remove(KEY_FIELD);
                       JsonEntity entity = new JsonEntity(entityName, key, array.getJSONObject(i));

                        cache.get(name).put(entity.key, entity);
                    }
                }
                catch (Message.EndOfBufferException | ExecutionException | InterruptedException | JSONException e)
                {
                    Common.logException(e);
                }
            });
            needInit = false;
        }
    }

    public static JsonEntityCache getInstance() {
        if (instance.needInit) instance.initializeCache();
        return instance;
    }

    public ConcurrentHashMap<String, ConcurrentHashMap<Integer, JsonEntity>> getCache()
    {
        return cache;
    }
}

