package com.cololo.tc.db.orm.provider;



import com.cololo.tc.Common;
import com.cololo.tc.server.Message;
import com.cololo.tc.server.Server;
import com.cololo.tc.tools.editor.WeakDataHolder;
import com.cololo.tc.tools.json.JSONArray;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ExecutionException;

public abstract class GateProvider<T>
{
    private final int gateCode;

    public GateProvider(int gateCode) {
        this.gateCode = gateCode;
    }


    public List<FilteredCallback<T>> getProviderListeners()
    {
        return WeakDataHolder.computeIfAbsent(this, CopyOnWriteArrayList::new);
    }

    public void addProviderListener(FilteredCallback<T> listener)
    {
        getProviderListeners().add(listener);
    }

    public void removeProviderListener(FilteredCallback<T> listener)
    {
        getProviderListeners().remove(listener);
    }

    protected void notifyProviderListeners(Event event, T t)
    {
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

    public T create(T object) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException
    {
        return jsonToData(Server.gateProviderCreateRequest(getGateCode(), dataToJson(object)));
    }

    public ArrayList<T> read(JsonFilter filter) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException
    {
        JSONArray array = Server.gateProviderReadRequest(getGateCode(), filter);
        ArrayList<T> result = new ArrayList<>();
        for (int i = 0; i < array.length(); i++) {
            result.add(jsonToData(array.getJSONObject(i)));
        }
        return result;
    }

    public void update(T object) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException
    {
        Server.gateProviderUpdateRequest(getGateCode(), dataToJson(object));
    }

    public void remove(T object) throws Message.EndOfBufferException, ExecutionException, InterruptedException, JSONException
    {
        Server.gateProviderRemoveRequest(getGateCode(), dataToJson(object));
    }

    protected int getGateCode()
    {
        return gateCode;
    }
    protected abstract JSONObject dataToJson(T data) throws JSONException;
    protected abstract T jsonToData(JSONObject json) throws JSONException;
}
