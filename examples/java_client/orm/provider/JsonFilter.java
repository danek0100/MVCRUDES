package com.cololo.tc.db.orm.provider;

import com.cololo.tc.Common;
import com.cololo.tc.db.orm.provider.jsonEntity.JsonEntity;
import com.cololo.tc.tools.DisplayCond;
import com.cololo.tc.tools.editor.IncompleteEntityException;
import com.cololo.tc.tools.json.JSONArray;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;

import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class JsonFilter
{
    public static final String FILTERS = "filters";

    public static final String FIELD_NAME = "fieldName";
    public static final String TYPE = "type";
    public static final String VALUE = "value";
    public static final String VALUES = "values";

    public static final String AND = "AND";
    public static final String OR = "OR";

    public static final String EQ = "EQ";
    public static final String NE = "NE";
    public static final String LT = "LT";
    public static final String LE = "LE";
    public static final String GT = "GT";
    public static final String GE = "GE";
    public static final String IN = "IN";
    public static final String NIN = "NIN"; // not in
    public static final String IS = "IS";
    public static final String NIS = "NIS"; // is not

    // for DisplayCondType
    public static final String CT = "CT";  // contains
    public static final String NC = "NC";  // not contains
    public static final String EP = "EP";  // Empty
    public static final String NP = "NP";  // not empty
    public static final String ALWAYS = "ALWAYS";
    public static final String NEVER = "NEVER";


    public JSONObject filter;
    private final static String wordPattern = "(?:[^\\\\\"]|\\\\\"|\\\\\\\\)";
    private final static Pattern patternFields = Pattern.compile(
            "([\\w%]+)\\s*" + // name of field
                    "(?:" + // start of group matching
                    "(!=|<=|>=|=|<|>)(?<![iI][nN])(?<![iI][sS])(?<![nN][oO][tT]\\s[iI][nN])(?<![iI][sS])(?<![iI][sS]\\s[nN][oO][tT])\\s*(\"(?:[^\\\\\"]|\\\\\"|\\\\\\\\)*\"|\\d+)|" + // math comparison operations and not "in", "is", "not in", "not is"
                    "([iI][nN])\\s*\\(((\\s*\\d+(?:\\s*,\\s*\\d+)*)|\"" + wordPattern + "*\"(?:\\s*,\\s*\"" + wordPattern + "*\")*\\s*)\\)|" + // "in" and array
                    "([nN][oO][tT]\\s[iI][nN])\\s*\\(((\\s*\\d+(?:\\s*,\\s*\\d+)*)|\"" + wordPattern + "*\"(?:\\s*,\\s*\"" + wordPattern + "*\")*\\s*)\\)|" + // "not in" and array
                    "([iI][sS])\\s*([nN][uU][lL][lL])|" + // "is null"
                    "([iI][sS]\\s[nN][oO][tT])\\s*([nN][uU][lL][lL])" + // "is not null"
                    ")"
    );

    private final static Pattern patternInStrings = Pattern.compile("(\"" + wordPattern +"*\")");
    private final static Pattern patternInInt = Pattern.compile("(\\d+)");

    private final static Pattern patternDeep = Pattern.compile("(\\(|!|[oO][rR]|[aA][nN][dD]|\\))");

    public JsonFilter(String filter) throws JSONException, IncompleteEntityException
    {
        if (filter == null || filter.trim().isEmpty())
        {
            this.filter = null;
            return;
        }

        final String REPLACE_FIELD_VALUE = "!";

        Matcher matcher = patternFields.matcher(filter);

        Queue<JSONObject> filtersQueue = new ArrayDeque<>();
        Map<String, String> condition = Map.of(
                "=", EQ,
                "!=", NE,
                "<", LT,
                "<=", LE,
                ">", GT,
                ">=", GE,
                "IN", IN,
                "NOT IN", NIN,
                "IS", IS,
                "IS NOT", NIS
        );
        while (matcher.find())
        {
            JSONObject jsonFilter = new JSONObject();

            jsonFilter.put(FIELD_NAME, matcher.group(1));
            if (matcher.group(2) != null) // math comparison operations
            {
                jsonFilter.put(TYPE, condition.get(matcher.group(2)));
                if (Common.isInt(matcher.group(3))) jsonFilter.put(VALUE, Integer.valueOf(matcher.group(3)));
                else if (matcher.group(3).length() == 2)jsonFilter.put(VALUE, "");
                else jsonFilter.put(VALUE, matcher.group(3).substring(1,matcher.group(3).length()-1).replace("\\\"", "\"").replace("\\\\", "\\"));
            }
            else if (matcher.group(4) != null) // in
            {
                JSONArray values = new JSONArray();
                jsonFilter.put(TYPE, condition.get(matcher.group(4).toUpperCase()));
                if (matcher.group(5).startsWith("\""))
                {
                    Matcher matcherStrings = patternInStrings.matcher(matcher.group(5));
                    while (matcherStrings.find())
                    {
                        String v = matcherStrings.group(1);
                        if (v.equals("\"\"")) values.put("");
                        else values.put( matcherStrings.group(1).substring(1,matcherStrings.group(1).length()-1));
                    }
                    jsonFilter.put(VALUES, values);
                }
                else
                {
                    Matcher matcherInt = patternInInt.matcher(matcher.group(5));
                    while (matcherInt.find()) values.put(Integer.valueOf(matcherInt.group(1)));
                    jsonFilter.put(VALUES, values);
                }
            }
            else if (matcher.group(7) != null) // not in
            {
                JSONArray values = new JSONArray();
                jsonFilter.put(TYPE, condition.get(matcher.group(7).toUpperCase()));
                if (matcher.group(8).startsWith("\""))
                {
                    Matcher matcherStrings = patternInStrings.matcher(matcher.group(8));
                    while (matcherStrings.find())
                    {
                        String v = matcherStrings.group(1);
                        if (v.equals("\"\"")) values.put("");
                        else values.put( matcherStrings.group(1).substring(1,matcherStrings.group(1).length()-1));
                    }
                    jsonFilter.put(VALUES, values);
                }
                else
                {
                    Matcher matcherInt = patternInInt.matcher(matcher.group(8));
                    while (matcherInt.find()) values.put(Integer.valueOf(matcherInt.group(1)));
                    jsonFilter.put(VALUES, values);
                }
            }
            else if (matcher.group(10) != null) // is
            {
                jsonFilter.put(TYPE, condition.get(matcher.group(10)));
                jsonFilter.put(VALUE,JSONObject.NULL);
            }
            else if (matcher.group(12) != null) // is not
            {
                jsonFilter.put(TYPE, condition.get(matcher.group(12)));
                jsonFilter.put(VALUE,JSONObject.NULL);
            }
            else
            {
                continue;
            }

            filtersQueue.add(jsonFilter);
        }

        if (filtersQueue.isEmpty())  throw new IncompleteEntityException(Common.getStr("JsonFilter.ParamError"));

        String replacedFilter =  matcher.replaceAll(e->REPLACE_FIELD_VALUE);

        Matcher deepMatcher = patternDeep.matcher(replacedFilter);

        Stack<JSONObject> children = new Stack<>();
        children.add(new JSONObject());

        while (deepMatcher.find())
        {
            String group = deepMatcher.group(0);

            if (group.equalsIgnoreCase("("))
            {
                children.add(new JSONObject());
            }
            else if (group.equalsIgnoreCase(REPLACE_FIELD_VALUE))
            {
                JSONObject currentRoot = children.peek();

                if (currentRoot.has(FILTERS)) // если уже есть условия
                {
                    if (!currentRoot.has(TYPE)) throw new IncompleteEntityException(Common.getStr("JsonFilter.ParamError"));

                    currentRoot.getJSONArray(FILTERS).put(filtersQueue.poll());
                }
                else
                {
                    currentRoot.put(FILTERS,new JSONArray());
                    currentRoot.getJSONArray(FILTERS).put(filtersQueue.poll());
                }
            }
            else if (group.equalsIgnoreCase(")"))
            {
                JSONObject currentRoot = children.pop();
                JSONObject parentRoot = children.peek();

                if (currentRoot.has(FILTERS))
                {
                    if (currentRoot.getJSONArray(FILTERS).length() == 1) currentRoot = currentRoot.getJSONArray(FILTERS).getJSONObject(0);
                }
                else if (!currentRoot.has(FIELD_NAME) ||!currentRoot.has(TYPE) || (!currentRoot.has(VALUE) && !currentRoot.has(VALUES))) throw new IncompleteEntityException(Common.getStr("JsonFilter.ParamError"));

                if (parentRoot.has(FILTERS))
                {
                    if (!parentRoot.has(TYPE)) throw new IncompleteEntityException(Common.getStr("JsonFilter.ParamError"));

                    parentRoot.getJSONArray(FILTERS).put(currentRoot);
                }
                else
                {
                    parentRoot.put(FILTERS,new JSONArray());
                    parentRoot.getJSONArray(FILTERS).put(currentRoot);
                }
            }
            else if (group.equalsIgnoreCase(AND) || group.equalsIgnoreCase(OR))
            {
                String groupType = group.toUpperCase();

                JSONObject currentRoot = children.peek();

                if (currentRoot.has(FILTERS))
                {
                    if (currentRoot.getJSONArray(FILTERS).length() == 0) throw new IncompleteEntityException(Common.getStr("JsonFilter.ParamError"));

                    if (currentRoot.getJSONArray(FILTERS).length() == 1)
                    {
                        if (!currentRoot.has(TYPE)) currentRoot.put(TYPE, groupType);
                    }
                    else if (!currentRoot.getString(TYPE).equals(groupType))
                    {
                        if (groupType.equalsIgnoreCase(AND))
                        {
                            JSONArray values = currentRoot.getJSONArray(FILTERS);
                            JSONObject lastValue = values.getJSONObject(values.length()-1);
                            values.remove(values.length()-1);
                            JSONObject newRoot = new JSONObject();
                            JSONArray filters = new JSONArray();
                            filters.put(lastValue);
                            newRoot.put(FILTERS, filters);
                            newRoot.put(TYPE, AND);
                            values.put(newRoot);

                            children.add(newRoot);
                        }
                        else
                        {
                            if (children.size() > 1 && children.get(children.size()-2).getString(TYPE).equals(OR))
                            {
                                children.pop();
                            }
                            else
                            {
                                currentRoot = children.pop();
                                JSONObject newRoot = new JSONObject();
                                JSONArray newFilters = new JSONArray();
                                newFilters.put(currentRoot);

                                newRoot.put(FILTERS, newFilters);
                                newRoot.put(TYPE, groupType);
                                children.add(newRoot);
                            }
                        }
                    }
                }
                else if (!currentRoot.has(FIELD_NAME) ||!currentRoot.has(TYPE) || (!currentRoot.has(VALUE) && !currentRoot.has(VALUES))) throw new IncompleteEntityException(Common.getStr("JsonFilter.ParamError"));

            }

            else throw new IncompleteEntityException(Common.getStr("JsonFilter.ParamError"));
        }

        JSONObject resultFilter = children.get(0);
        if (!resultFilter.has(TYPE) && resultFilter.getJSONArray(FILTERS).length() == 1) resultFilter = resultFilter.getJSONArray(FILTERS).getJSONObject(0);

        this.filter = resultFilter;
    }

    public JsonFilter(JSONObject filter)
    {
        this.filter = filter;
    }

    public JsonFilter(DisplayCond cond) throws JSONException, IncompleteEntityException {
        this(cond.getHardUserCond());
        if(cond.getDisplayCondType() != DisplayCond.DisplayCondType.USER_HARD)
        {
            JSONObject json = new JSONObject();
            json.put(TYPE, cond.getDisplayCondType().convertToJsonString());
            if(cond.getDisplayCondType() != DisplayCond.DisplayCondType.NEVER
                    && cond.getDisplayCondType() != DisplayCond.DisplayCondType.ALWAYS) {
                json.put(FIELD_NAME, cond.getCondSearchWhere());
                if(cond.getDisplayCondType() != DisplayCond.DisplayCondType.IF_EMPTY
                        && cond.getDisplayCondType() != DisplayCond.DisplayCondType.IF_NOT_EMPTY)
                    json.put(VALUE, cond.getCondSearchFor());
            }
            this.filter = json;
        }
    }

    public boolean isFiltered(JsonEntity data) throws JSONException
    {
        if (filter == null) return true;
        JSONObject checkData = new JSONObject(data.value.toString());
        checkData.put("ID", data.key);

        if (filter.has(FILTERS)) return checkFilterGroup(filter, checkData);
        else return checkField(filter, checkData);
    }

    public boolean isFiltered(JSONObject data) throws JSONException
    {
        if (filter == null) return true;

        if (filter.has(FILTERS)) return checkFilterGroup(filter, data);
        else return checkField(filter, data);
    }

    boolean checkFilterGroup(JSONObject groupFilter, JSONObject data) throws JSONException
    {
        String type = groupFilter.getString(TYPE);


        if (AND.equals(type))
        {
            JSONArray array = filter.getJSONArray(FILTERS);
            for (int i = 0; i < array.length(); i++)
            {
                JSONObject item = array.getJSONObject(i);
                if (!item.isNull(FILTERS))
                {
                    if (!checkFilterGroup(item.getJSONObject(FILTERS), data)) return false;
                }
                else
                {
                    if (!checkField(item, data)) return false;
                }
            }

            return true;
        }
        else if (OR.equals(type))
        {
            JSONArray array = filter.getJSONArray(FILTERS);
            for (int i = 0; i < array.length(); i++)
            {
                JSONObject item = array.getJSONObject(i);
                if (!item.isNull(FILTERS))
                {
                    if (checkFilterGroup(item.getJSONObject(FILTERS), data)) return true;
                }
                else
                {
                    if (checkField(item, data)) return true;
                }
            }

            return false;
        }

        return false;
}

    boolean checkField(JSONObject fieldFilter, JSONObject data) throws JSONException
    {
        String fieldName = fieldFilter.getString(FIELD_NAME);
        String type = fieldFilter.getString(TYPE);

        if (EQ.equals(type))
        {
            return data.get(fieldName).equals(fieldFilter.get(VALUE));
        }
        else if (NE.equals(type))
        {
            return !data.get(fieldName).equals(fieldFilter.get(VALUE));
        }
        else if (LT.equals(type))
        {
            return data.getLong(fieldName) < (fieldFilter.getLong(VALUE));
        }
        else if (LE.equals(type))
        {
            return data.getLong(fieldName) <= (fieldFilter.getLong(VALUE));
        }
        else if (GT.equals(type))
        {
            return data.getLong(fieldName) > (fieldFilter.getLong(VALUE));
        }
        else if (GE.equals(type))
        {
            return data.getLong(fieldName) >= (fieldFilter.getLong(VALUE));
        }
        else if (IN.equals(type))
        {
            JSONArray inValues = fieldFilter.getJSONArray(VALUES);
            for (int i = 0; i < inValues.length(); i++)
            {
                if(data.get(fieldName).equals(inValues.get(i))) return true;
            }
        }
        else if (NIN.equals(type))
        {
            JSONArray inValues = fieldFilter.getJSONArray(VALUES);
            for (int i = 0; i < inValues.length(); i++)
            {
                if(data.get(fieldName).equals(inValues.get(i))) return false;
            }
            return true;
        }
        else if (IS.equals(type))
        {
            return data.getString(fieldName).equalsIgnoreCase("NULL"); // applies only to NULL
        }
        else if (NIS.equals(type))
        {
            return !data.getString(fieldName).equalsIgnoreCase("NULL"); // applies only to NULL
        }

        return false;
    }

    public DisplayCond parseCondition(){
        DisplayCond result = new DisplayCond();

        if(filter.has(FILTERS)){
           result.setDisplayCondType(DisplayCond.DisplayCondType.USER_HARD);
           result.setHardUserCond(parseFilter(filter));
        } else {
            try {
                result.setDisplayCondType(DisplayCond.DisplayCondType.fromJsonString(filter.get(TYPE).toString()));
                if(result.getDisplayCondType() != DisplayCond.DisplayCondType.ALWAYS
                        && result.getDisplayCondType() != DisplayCond.DisplayCondType.NEVER) {
                    result.setCondSearchWhere(filter.get(FIELD_NAME).toString());
                    result.setCondSearchFor(filter.get(VALUE).toString());
                }
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
        return result;
    }

    private String parseFilter(JSONObject filter){
        StringBuilder result = new StringBuilder();
        try {
            JSONArray array = filter.getJSONArray(FILTERS);
            String type = filter.getString(TYPE); // AND or OR

            for(int i = 0; i < array.length(); i++){
                JSONObject o =  (JSONObject) array.get(i);
                if(o.has(FILTERS)){
                    if(filter.get(TYPE).equals(AND) && o.get(TYPE).equals(OR))
                        result.append("(");
                    result.append(parseFilter(o));
                    if(filter.get(TYPE).equals(AND) && o.get(TYPE).equals(OR))
                        result.append(")");
                    if (i < array.length()-1) {
                        result.append(" ");
                        result.append(type);
                        result.append(" ");
                    }
                } else {
                    result.append(o.get(FIELD_NAME));
                    result.append(" ");
                    result.append(typeFromJsonToString(o.get(TYPE).toString()));
                    result.append(" ");
                    result.append((o.get(VALUE) instanceof String) ? "\"" + o.get(VALUE) + "\"" : o.get(VALUE));
                    if (i < array.length()-1) {
                        result.append(" ");
                        result.append(type);
                        result.append(" ");
                    }
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return result.toString();
    }

    private String typeFromJsonToString(String type){
        switch (type) {
            case EQ -> {
                return "=";
            }
            case NE -> {
                return "!=";
            }
            case LE -> {
                return "<=";
            }
            case LT -> {
                return "<";
            }
            case GE -> {
                return ">=";
            }
            case GT -> {
                return ">";
            }
            case IN -> {
                return "IN";
            }
        }
        return type;
    }

    @Override
    public String toString()
    {
        return filter.toString();
    }
}
