package com.cololo.tc.db.orm.jsonrepository;

import com.cololo.tc.Common;
import com.cololo.tc.db.orm.jsonrepository.exception.JsonCrudException;
import com.cololo.tc.db.orm.jsonrepository.mapper.JsonEntityMapper;
import com.cololo.tc.db.orm.jsonrepository.exception.JsonEntityMappingException;
import com.cololo.tc.db.orm.jsonrepository.mapper.Mapper;
import com.cololo.tc.db.orm.jsonrepository.mapper.UpperCamelCaseMapper;
import com.cololo.tc.db.orm.provider.JsonFilter;
import com.cololo.tc.db.orm.provider.jsonEntity.JsonEntity;
import com.cololo.tc.db.orm.provider.jsonEntity.JsonEntityProvider;
import com.cololo.tc.server.Message;
import com.cololo.tc.tools.editor.IncompleteEntityException;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;

import java.util.List;
import java.util.Optional;
import java.util.concurrent.ExecutionException;

/**
 * Репозиторий для работы с JsonEntity как c Java-объектами.
 * @param <T> Тип сущности (Java).
 */
public class JsonRepository<T> implements CrudRepository<T> {
    private final Mapper<String, String> classNameConverter = new UpperCamelCaseMapper();
    private final JsonEntityProvider jsonEntityProvider = JsonEntityProvider.getInstance();
    private final Mapper<JsonEntity, T> jsonEntityMapper;
    private final Class<T> clazz;

    public JsonRepository(Class<T> clazz) {
        try {
            this.jsonEntityMapper = new JsonEntityMapper<>(clazz);
        } catch (JsonEntityMappingException e) {
            Common.logException(e);
            Common.fatalErrorNoOwner(Common.getStr("JsonRepository.initError"));
            throw new RuntimeException(e);
        }

        this.clazz = clazz;
    }

    /**
     * Сохранение сущности. Если key == -1, то сохраняет, иначе обновляет существующую.
     * @param entity Сущность.
     * @return Сохраненная сущность с заданным key.
     * @throws JsonCrudException В случае ошибки.
     */
    @Override
    public T save(T entity) throws JsonCrudException {
        T result;
        
        try {
            JsonEntity jsonEntity = jsonEntityMapper.mapTo(entity);

            if (jsonEntity.key == -1) {
                JsonEntity jsonResult = jsonEntityProvider.create(jsonEntity);
                result = jsonEntityMapper.mapFrom(jsonResult);
            } else {
                jsonEntityProvider.update(jsonEntity);
                result = entity;
            }

        } catch (JSONException | Message.EndOfBufferException | ExecutionException | InterruptedException e) {
            throw new JsonCrudException(e);
        }

        return result;
    }

    /**
     * Чтение с фильтром.
     * @param filter Фильтр сущностей формата JsonFilter.
     * @return Список сущностей.
     * @throws JsonCrudException В случае ошибки.
     */
    @Override
    public List<T> read(String filter) throws JsonCrudException {
        String className  = clazz.getSimpleName();
        String entityName = classNameConverter.mapFrom(className).toUpperCase();

        List<T> result;

        try {
            JsonFilter jsonFilter = new JsonFilter(filter);
            result = jsonEntityProvider.read(entityName, jsonFilter)
                    .stream()
                    .map(jsonEntityMapper::mapFrom)
                    .toList();
        } catch (JSONException | Message.EndOfBufferException | ExecutionException | IncompleteEntityException | InterruptedException e) {
            throw new JsonCrudException(e);
        }

        return result;
    }

    /**
     * Удаление сущности.
     * @param entity Сущность.
     * @throws JsonCrudException В случае ошибки.
     */
    @Override
    public void delete(T entity) throws JsonCrudException {
        try {
            JsonEntity jsonEntity = jsonEntityMapper.mapTo(entity);
            jsonEntityProvider.remove(jsonEntity);
        } catch (JSONException | Message.EndOfBufferException | ExecutionException | InterruptedException e) {
            throw new JsonCrudException(e);
        }
    }

    /**
     * Удаление сущностей по фильтру.
     * @param filter Фильтр сущностей формата JsonFilter.
     * @throws JsonCrudException В случае ошибки.
     */
    @Override
    public void delete(String filter) throws JsonCrudException {
        List<T> toDelete = read(filter);
        for (T entity : toDelete) {
            delete(entity);
        }
    }
}