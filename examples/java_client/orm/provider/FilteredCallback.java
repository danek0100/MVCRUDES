package com.cololo.tc.db.orm.provider;

public abstract class FilteredCallback<T>
{
    public final JsonFilter filter;

    protected FilteredCallback(JsonFilter filter)
    {
        this.filter = filter;
    }

    public abstract void call(Event event, T t);
}
