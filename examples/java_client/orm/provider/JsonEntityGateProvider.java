package com.cololo.tc.db.orm.provider;


import com.cololo.tc.Common;
import com.cololo.tc.db.orm.provider.jsonEntity.JsonEntityCache;
import com.cololo.tc.server.Message;
import com.cololo.tc.server.Server;
import com.cololo.tc.tools.editor.WeakDataHolder;
import com.cololo.tc.tools.json.JSONArray;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;

import java.util.*;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ExecutionException;
import java.util.stream.Collectors;

/**
 * Abstract class that provides a generic mechanism for managing entities through gate communication.
 * It supports basic CRUD operations and maintains a list of listeners for entity-related events.
 *
 * @param <T> The type of entity this provider will manage.
 */
public abstract class JsonEntityGateProvider<T> {
    private final int gateCode;

    /**
     * Constructor for JsonEntityGateProvider.
     *
     * @param gateCode The gate code used for server communication related to entities.
     */
    public JsonEntityGateProvider(int gateCode) {
        this.gateCode = gateCode;
    }

    /**
     * Returns a list of provider listeners.
     *
     * @return List of {@link FilteredCallback<T>} associated with this provider.
     */
    public List<FilteredCallback<T>> getProviderListeners() {
        return WeakDataHolder.computeIfAbsent(this, CopyOnWriteArrayList::new);
    }

    /**
     * Adds a new provider listener.
     *
     * @param listener The listener to be added.
     */
    public void addProviderListener(FilteredCallback<T> listener) {
        getProviderListeners().add(listener);
    }

    /**
     * Removes a provider listener.
     *
     * @param listener The listener to be removed.
     */
    public void removeProviderListener(FilteredCallback<T> listener) {
        getProviderListeners().remove(listener);
    }

    /**
     * Notifies all provider listeners about a specific event related to an entity.
     *
     * @param event The event that occurred.
     * @param t     The entity related to the event.
     */
    protected void notifyProviderListeners(Event event, T t) {
        getProviderListeners().forEach(filteredCallback ->
        {
            try
            {
                if (filteredCallback.filter.isFiltered(dataToJson(t))) filteredCallback.call(event, t);
            }
            catch (JSONException e)
            {
                Common.logException(e);
            }
        });
    }

    /**
     * Creates a new entity in the system.
     *
     * @param object The entity to be created.
     * @return The created entity.
     * @throws Message.EndOfBufferException If there is a buffer error during message processing.
     * @throws ExecutionException           If the execution is interrupted or fails.
     * @throws InterruptedException         If the thread is interrupted while waiting.
     * @throws JSONException                If there is a JSON parsing error.
     */
    public T create(T object) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException {
        List<T> created = create(Collections.singletonList(object));
        if (created.size() > 0) {
            return created.get(0);
        }
        return null;
    }

    /**
     * Creates a new entity in the system.
     *
     * @param objects The entities to be created.
     * @return The created entities.
     * @throws Message.EndOfBufferException If there is a buffer error during message processing.
     * @throws ExecutionException           If the execution is interrupted or fails.
     * @throws InterruptedException         If the thread is interrupted while waiting.
     * @throws JSONException                If there is a JSON parsing error.
     */
    public List<T> create(List<T> objects) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException {
        JSONArray objectsArray = new JSONArray();
        for (T object : objects) {
            objectsArray.put(dataToJson(object));
        }
        JSONArray createdArray = Server.gateProviderEntitiesCreateRequest(getGateCode(), objectsArray);
        List<T> createdList = new ArrayList<>();
        for (int i = 0; i < createdArray.length(); ++i) {
            createdList.add(jsonToData(createdArray.getJSONObject(i)));
        }
        return createdList;
    }

    /**
     * Reads entities based on the specified type and filter.
     *
     * @param name   The name of entities to read.
     * @param filter The filter to apply on entities.
     * @return List of entities that match the criteria.
     * @throws Message.EndOfBufferException If there is a buffer error during message processing.
     * @throws ExecutionException           If the execution is interrupted or fails.
     * @throws InterruptedException         If the thread is interrupted while waiting.
     * @throws JSONException                If there is a JSON parsing error.
     */
    public ArrayList<T> read(String name, JsonFilter filter) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException {
        return read(name, filter, null);
    }

    /**
     * Reads entities based on the specified type and filter.
     *
     * @param name   The name of entities to read.
     * @param filter The filter to apply on entities.
     * @param fields The fields to read.
     * @return List of entities that match the criteria.
     * @throws Message.EndOfBufferException If there is a buffer error during message processing.
     * @throws ExecutionException           If the execution is interrupted or fails.
     * @throws InterruptedException         If the thread is interrupted while waiting.
     * @throws JSONException                If there is a JSON parsing error.
     */
    public ArrayList<T> read(String name, JsonFilter filter, JSONArray fields) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException
    {
        if (JsonEntityCache.getInstance().getCache().get(name) != null)
        {
            return (ArrayList<T>)JsonEntityCache.getInstance().getCache().get(name).values().stream().filter(e->Common.getOrDefaultSafe(()->filter.isFiltered(e), false)).collect(Collectors.toCollection(ArrayList::new));
        }
        JSONArray array = Server.gateProviderEntitiesReadRequest(getGateCode(), name, filter, fields);
        ArrayList<T> result = new ArrayList<>();
        for (int i = 0; i < array.length(); i++) {
            result.add(jsonToData(array.getJSONObject(i)));
        }
        return result;
    }

    /**
     * Updates an existing entity.
     *
     * @param object The entity to be updated.
     * @throws Message.EndOfBufferException If there is a buffer error during message processing.
     * @throws ExecutionException           If the execution is interrupted or fails.
     * @throws InterruptedException         If the thread is interrupted while waiting.
     * @throws JSONException                If there is a JSON parsing error.
     */
    public void update(T object) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException {
        update(Collections.singletonList(object));
    }

    /**
     * Updates existing entities.
     *
     * @param objects The entities to be updated.
     * @throws Message.EndOfBufferException If there is a buffer error during message processing.
     * @throws ExecutionException           If the execution is interrupted or fails.
     * @throws InterruptedException         If the thread is interrupted while waiting.
     * @throws JSONException                If there is a JSON parsing error.
     */
    public void update(Collection<T> objects) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException {
        JSONArray objectsArray = new JSONArray();
        for (T object : objects) {
            objectsArray.put(dataToJson(object));
        }
        Server.gateProviderEntitiesUpdateRequest(getGateCode(), objectsArray);
    }

    /**
     * Removes an entity from the system.
     *
     * @param object The entity to be removed.
     * @throws Message.EndOfBufferException If there is a buffer error during message processing.
     * @throws ExecutionException           If the execution is interrupted or fails.
     * @throws InterruptedException         If the thread is interrupted while waiting.
     * @throws JSONException                If there is a JSON parsing error.
     */
    public void remove(T object) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException {
        remove(Collections.singletonList(object));
    }

    /**
     * Removes an entities from the system.
     *
     * @param objects The entities to be removed.
     * @throws Message.EndOfBufferException If there is a buffer error during message processing.
     * @throws ExecutionException           If the execution is interrupted or fails.
     * @throws InterruptedException         If the thread is interrupted while waiting.
     * @throws JSONException                If there is a JSON parsing error.
     */
    public void remove(List<T> objects) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException {
        JSONArray objectsArray = new JSONArray();
        for (T object : objects) {
            objectsArray.put(dataToJson(object));
        }
        Server.gateProviderEntitiesRemoveRequest(getGateCode(), objectsArray);
    }

   public void remove(String name, JsonFilter filter) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException
   {
       JSONArray fields = new JSONArray();
       fields.put("ID");
       ArrayList<T> data = read(name, filter, fields);

       remove(data);
   }

    /**
     * Gets the gate code associated with this provider.
     *
     * @return The gate code.
     */
    protected int getGateCode() {
        return gateCode;
    }

    /**
     * Converts data to a JSONObject.
     *
     * @param data The data to be converted.
     * @return JSONObject representation of the data.
     * @throws JSONException If there is a JSON parsing error.
     */
    protected abstract JSONObject dataToJson(T data) throws JSONException;

    /**
     * Converts a JSONObject to data.
     *
     * @param json The JSONObject to be converted.
     * @return Data representation of the JSONObject.
     * @throws JSONException If there is a JSON parsing error.
     */
    protected abstract T jsonToData(JSONObject json) throws JSONException;
}
