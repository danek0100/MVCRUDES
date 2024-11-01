package com.cololo.tc.db.orm.provider.jsonEntity;

import com.cololo.tc.Common;
import com.cololo.tc.db.orm.provider.JsonEntityGateProvider;
import com.cololo.tc.db.orm.provider.Event;
import com.cololo.tc.server.Message;
import com.cololo.tc.server.Server;
import com.cololo.tc.server.ServerAsyncListener;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;

import java.util.concurrent.ExecutionException;

import static com.cololo.tc.db.orm.provider.jsonEntity.JsonEntity.ENTITY_NAME_FIELD;
import static com.cololo.tc.db.orm.provider.jsonEntity.JsonEntity.KEY_FIELD;
import static com.cololo.tc.server.Server.SUBSCRIBE;
import static com.cololo.tc.server.ServerProtocol.GATE_JSONENTITY_PROVIDER;

/**
 * Provides entities for a specific JSON entity type, handling subscription to server updates.
 */
public class JsonEntityProvider extends JsonEntityGateProvider<JsonEntity> {

    private final static JsonEntityProvider instance = new JsonEntityProvider();
    private boolean needInit = true;

    private JsonEntityProvider() {
        super(GATE_JSONENTITY_PROVIDER);
        Server.get().addConnectionStateListener(() -> {
            if (!Server.get().isConnected()) {
                needInit = true;
            }
        });
    }

    private synchronized void init() {
        if (Server.get().isConnected() && needInit) {
            try {
                setupServerListener();
                needInit = false;
            } catch (Message.EndOfBufferException | ExecutionException | InterruptedException e) {
                Common.logException(e);
            }

        }
    }

    private void setupServerListener() throws Message.EndOfBufferException, ExecutionException, InterruptedException {
        ServerAsyncListener serverAsyncListener = this::handleServerMessage;
        Server.gateProviderSpeakerRequest(getGateCode(), SUBSCRIBE, null, serverAsyncListener);
    }

    private void handleServerMessage(Message message) {
        try {
            JSONObject jsonObject = new JSONObject(message.extractStr());
            notifyProviderListeners(Event.valueOf(jsonObject.getString("event")), jsonToData(jsonObject.getJSONObject("object")));
        } catch (Exception e) {
            Common.logException(e);
        }
    }

    /**
     * Converts an JsonEntity object to its JSONObject representation.
     * This method takes an JsonEntity object, extracts its properties,
     * and returns a JSONObject that represents the same data.
     * The 'entityName' property is added to the JSON object.
     *
     * @param data the JsonEntity object to convert to JSON.
     * @return JSONObject representing the data of the provided JsonEntity object.
     * @throws JSONException if there is an error during JSON manipulation.
     */
    @Override
    public JSONObject dataToJson(JsonEntity data) throws JSONException {
        return data.value.put(ENTITY_NAME_FIELD, data.name).put(KEY_FIELD, data.key);
    }

    /**
     * Converts a JSONObject into an JsonEntity object.
     * This method creates an JsonEntity object using data extracted from
     * the provided JSONObject. It removes 'entityName' from the JSON object
     * and uses it along with the 'ID' field to construct the JsonEntity.
     *
     * @param json the JSONObject to convert into an JsonEntity object.
     * @return an JsonEntity object constructed from the JSONObject.
     * @throws JSONException if there is an error during JSON manipulation.
     */
    @Override
    public JsonEntity jsonToData(JSONObject json) throws JSONException {
        String entityName = (String) json.remove(ENTITY_NAME_FIELD);
        int key = (Integer) json.remove(KEY_FIELD);
        return new JsonEntity(entityName, key, json);
    }

    /**
     * @return the single instance of JsonEntityProvider
     */
    public static JsonEntityProvider getInstance() {
        if (instance.needInit) instance.init();
        return instance;
    }
}
