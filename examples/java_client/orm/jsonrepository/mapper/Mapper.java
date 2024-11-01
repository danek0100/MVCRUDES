package com.cololo.tc.db.orm.jsonrepository.mapper;

/**
 * Конвертер объектов двух типов
 * @param <T> Тип, в который конвертировать
 * @param <S> Тип, из которого конвертировать
 */
public interface Mapper<T, S> {
    T mapTo(S source);
    S mapFrom(T source);
}
