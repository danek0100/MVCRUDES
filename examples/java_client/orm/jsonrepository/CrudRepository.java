package com.cololo.tc.db.orm.jsonrepository;

import com.cololo.tc.db.orm.jsonrepository.exception.CrudException;

import java.util.List;

/**
 * Базовый CRUD-репозиторий.
 * @param <T> Тип сущности.
 */
public interface CrudRepository<T> {
    /**
     * Сохранение сущности. Если такая сущность уже есть, выполняет обновление.
     * @param entity Сущность.
     * @return Сохраненная сущность.
     * @throws CrudException В случае ошибки.
     */
    T save(T entity) throws CrudException;

    /**
     * Чтение сущностей по фильтру.
     * @param filter Фильтр сущностей.
     * @return Сущности.
     * @throws CrudException В случае ошибки.
     */
    List<T> read(String filter) throws CrudException;

    /**
     * Удаление единичной сущности.
     * @param entity Сущность.
     * @throws CrudException В случае ошибки.
     */
    void delete(T entity) throws CrudException;

    /**
     * Удаление сущностей по фильтру.
     * @param filter Фильтр.
     * @throws CrudException В случае ошибки.
     */
    void delete(String filter) throws CrudException;
}
