package com.cololo.tc.db.orm.jsonrepository.mapper;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

public class UpperCamelCaseMapperTest {
    private final Mapper<String, String> mapper = new UpperCamelCaseMapper();

    @Test
    public void mapTo() {
        String[] src = {
                "test_string",
                "test_string_",
                "_test_string",
                "TEST_STRING",
                "teststring",
                "testString",
                "t"
        };

        String[] exp = {
                "TestString",
                "TestString",
                "TestString",
                "TestString",
                "Teststring",
                "Teststring",
                "T"
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
                "TESTSTRING"

        };

        String[] exp = {
                "test_string",
                "test_string",
                "teststring",
                "teststring",
        };

        for (int i = 0; i < src.length; i++) {
            String act = mapper.mapFrom(src[i]);
            assertEquals(exp[i], act);
        }
    }
}
