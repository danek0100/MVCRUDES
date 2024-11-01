package com.cololo.tc.db.orm.provider;

import com.cololo.tc.tools.DisplayCond;
import com.cololo.tc.tools.editor.IncompleteEntityException;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.EnumSource;
import org.junit.jupiter.params.provider.ValueSource;

import java.util.ArrayList;

import static org.junit.jupiter.api.Assertions.*;

public class DisplayCondToJsonTest {

    static ArrayList<String> conditions = new ArrayList<>();
    static {
        conditions.add("=");
        conditions.add("!=");
        conditions.add("<");
        conditions.add("<=");
        conditions.add(">");
        conditions.add(">=");
    }

    @Test
    public void alwaysOrNeverCondsToJson() throws JSONException, IncompleteEntityException {
        DisplayCond alwaysCond = new DisplayCond(DisplayCond.DisplayCondType.ALWAYS, "","");
        DisplayCond neverCond = new DisplayCond(DisplayCond.DisplayCondType.NEVER, "","");

        JSONObject alwaysObj = new JSONObject();
        alwaysObj.put(JsonFilter.TYPE, DisplayCond.DisplayCondType.ALWAYS.convertToJsonString());

        JSONObject neverObj = new JSONObject();
        neverObj.put(JsonFilter.TYPE, DisplayCond.DisplayCondType.NEVER.convertToJsonString());

        assertEquals(new JsonFilter(alwaysObj).toString(), new JsonFilter(alwaysCond).toString());
        assertEquals(new JsonFilter(neverObj).toString(), new JsonFilter(neverCond).toString());
    }


    @ParameterizedTest
    @EnumSource(DisplayCond.DisplayCondType.class)
    public void displayCondToJson(DisplayCond.DisplayCondType type) throws JSONException, IncompleteEntityException {
        if (type.equals(DisplayCond.DisplayCondType.USER_HARD)) {
            return;
        }
        DisplayCond condition;
        JSONObject condObj;
        String forString = "FOR";
        String whereString = "WHERE";

        condition = new DisplayCond(type, forString, whereString);
        condObj = new JSONObject();
        condObj.put(JsonFilter.TYPE, type.convertToJsonString());
        if (!type.equals(DisplayCond.DisplayCondType.ALWAYS) && !type.equals(DisplayCond.DisplayCondType.NEVER)) {
            condObj.put(JsonFilter.FIELD_NAME, whereString);
            if (!type.equals(DisplayCond.DisplayCondType.IF_EMPTY) && !type.equals(DisplayCond.DisplayCondType.IF_NOT_EMPTY))
                condObj.put(JsonFilter.VALUE, forString);
        }
        assertEquals(new JsonFilter(condition).toString(), new JsonFilter(condObj).toString());
    }

    @Test
    public void userCondToJson() throws JSONException, IncompleteEntityException {
        String userCond = "(ID > 10 OR ID != 15) AND ID <= 100 OR NAME = \"Administrator\"";
        String userCondJson = "{\"filters\":[{\"filters\":[{\"filters\":[{\"fieldName\":\"ID\",\"type\":\"GT\",\"value\":10},{\"fieldName\":\"ID\",\"type\":\"NE\",\"value\":15}],\"type\":\"OR\"},{\"fieldName\":\"ID\",\"type\":\"LE\",\"value\":100}],\"type\":\"AND\"},{\"fieldName\":\"NAME\",\"type\":\"EQ\",\"value\":\"Administrator\"}],\"type\":\"OR\"}";
        assertEquals(new JsonFilter(userCond).toString(), userCondJson);
        assertEquals(new JsonFilter(new DisplayCond(userCond)).toString(), userCondJson);
    }

    @Test
    public void jsonCondToDisplayCond() throws JSONException {
        String userCond = "(ID > 10 OR ID != 15) AND ID <= 100 OR NAME = \"Administrator\"";
        String userCondJson = "{\"filters\":[{\"filters\":[{\"filters\":[{\"fieldName\":\"ID\",\"type\":\"GT\",\"value\":10},{\"fieldName\":\"ID\",\"type\":\"NE\",\"value\":15}],\"type\":\"OR\"},{\"fieldName\":\"ID\",\"type\":\"LE\",\"value\":100}],\"type\":\"AND\"},{\"fieldName\":\"NAME\",\"type\":\"EQ\",\"value\":\"Administrator\"}],\"type\":\"OR\"}";
        String alwaysCondJson = "{\"type\":\"ALWAYS\"}";
        DisplayCond alwaysCond = new DisplayCond(DisplayCond.DisplayCondType.ALWAYS, "", "");
        String neverCondJson = "{\"type\":\"NEVER\"}";
        DisplayCond neverCond = new DisplayCond(DisplayCond.DisplayCondType.NEVER, "", "");
        String otherCondJson = "{\"fieldName\":\"ID\",\"type\":\"NE\",\"value\":15}";
        DisplayCond otherCond = new DisplayCond(DisplayCond.DisplayCondType.IF_NOT_EQUAL, "15", "ID");

        assertEquals(new JsonFilter(new JSONObject(userCondJson)).parseCondition().getHardUserCond(), userCond);
        assertEquals(new JsonFilter(new JSONObject(alwaysCondJson)).parseCondition(), alwaysCond);
        assertEquals(new JsonFilter(new JSONObject(neverCondJson)).parseCondition(), neverCond);
        assertEquals(new JsonFilter(new JSONObject(otherCondJson)).parseCondition(), otherCond);
    }
}
