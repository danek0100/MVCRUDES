package com.cololo.tc.db.orm.jsonrepository.mapper;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

public class LowerCamelCaseMapperTest {
    private final Mapper<String, String> mapper = new LowerCamelCaseMapper();

    @Test
    public void mapTo() {
        String[] src = {
                "test_string",
                "test_string_",
                "_test_string",
                "TEST_STRING",
                "teststring",
                "testString",
                ""
        };

        String[] exp = {
                "testString",
                "testString",
                "TestString",
                "testString",
                "teststring",
                "teststring",
                ""
        };

        for (int i = 0; i < src.length; i++) {
            String act = mapper.mapTo(src[i]);
            assertEquals(exp[i], act);
        }
    }

    @Test
    public void mapFrom() {
        String[] src = {
                "testString",
                "TestString",
                "teststring",
                "TESTSTRING",
                ""
        };

        String[] exp = {
                "test_string",
                "test_string",
                "teststring",
                "teststring",
                ""
        };

        for (int i = 0; i < src.length; i++) {
            String act = mapper.mapFrom(src[i]);
            assertEquals(exp[i], act);
        }
    }
}
