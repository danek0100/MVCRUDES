package com.cololo.tc.db.orm.jsonrepository.exception;

/**
 * Ошибка конвертирования между JsonEntity и Java-объектом.
 */
public class JsonEntityMappingException extends RuntimeException {
    public JsonEntityMappingException(String message) {
        super(message);
    }
}
