package com.cololo.tc.db.orm.cache;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

public class EntityCache
{
    /**
     * Класс хранящихся объектов -> id -> объект
     */
    private static final Map<Class<?>, Map<Integer, Object>> cache = new HashMap<>();

    public static <T> T get(Class<T> entityClass, int entityId)
    {
        synchronized (cache)
        {
            Object entity = cache.computeIfAbsent(entityClass, k -> new HashMap<>()).get(entityId);
            if (entity == null) return null;
            return entityClass.cast(entity);
        }
    }

    public static <T> void put(Class<T> entityClass, int id, T entity)
    {
        synchronized (cache)
        {
            cache.computeIfAbsent(entityClass, k-> new HashMap<>()).put(id, entity);
        }
    }

    public static <T> void remove(Class<T> entityClass, int id)
    {
        synchronized (cache)
        {
            cache.computeIfAbsent(entityClass, k -> new HashMap<>()).remove(id);
        }
    }

    public static <T> void clear(Class<T> entityClass, Collection<Integer> ids)
    {
        synchronized (cache)
        {
            Map<Integer,Object> cacheClass = cache.get(entityClass);
            if (cacheClass == null || cacheClass.isEmpty()) return;

            for (int id : ids)
            {
                cacheClass.remove(id);
            }
        }
    }

    public static <T> void clear(Class<T> entityClass)
    {
        synchronized (cache)
        {
            cache.computeIfAbsent(entityClass, k -> new HashMap<>()).clear();
        }
    }

    public static void clearAll()
    {
        synchronized (cache)
        {
            cache.clear();
        }
    }
}
