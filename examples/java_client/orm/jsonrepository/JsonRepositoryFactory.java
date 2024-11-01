package com.cololo.tc.db.orm.jsonrepository;

import java.util.HashMap;
import java.util.Map;

/**
 * Фабрика Json-репозиториев. Лениво предоставляет репозитории для сущностей любых типов.
 */
public abstract class JsonRepositoryFactory {
    private static final Map<Class<?>, JsonRepository<?>> repositoryHolder = new HashMap<>();

    /**
     * @param clazz Класс сущности.
     * @return Репозиторий.
     * @param <T> Тип сущности.
     * @throws RuntimeException Ошибка инициализации репозитория.
     */
    @SuppressWarnings("unchecked")
    public static <T> CrudRepository<T> getRepository(Class<T> clazz) {
        JsonRepository<T> result;
        synchronized (JsonRepositoryFactory.class) {
            if (repositoryHolder.containsKey(clazz)) {
                result = (JsonRepository<T>) repositoryHolder.get(clazz);
            }
            else {
                result = new JsonRepository<>(clazz);
                repositoryHolder.put(clazz, result);
            }
        }
        return result;
    }
}