package com.cololo.tc.db.orm.jsonrepository;

import com.cololo.tc.db.orm.jsonrepository.mapper.JsonEntityMapper;

import java.util.HashMap;
import java.util.Map;

/**
 * @brief Холдер мапперов JsonEntity в сущности.
 */
public abstract class JsonEntityMappers {
    private static final Map<Class<?>, JsonEntityMapper<?>> holder = new HashMap<>();

    /**
     * @param clazz Класс сущности.
     * @return Маппер.
     * @param <T> Тип сущности.
     * @throws RuntimeException Ошибка инициализации репозитория.
     */
    @SuppressWarnings("unchecked")
    public static <T> JsonEntityMapper<T> get(Class<T> clazz) {
        JsonEntityMapper<T> result;
        synchronized (JsonRepositoryFactory.class) {
            if (holder.containsKey(clazz)) {
                result = (JsonEntityMapper<T>) holder.get(clazz);
            }
            else {
                result = new JsonEntityMapper<>(clazz);
                holder.put(clazz, result);
            }
        }
        return result;
    }
}
