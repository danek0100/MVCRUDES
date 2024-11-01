package com.cololo.tc.db.orm.provider;

import com.cololo.tc.tools.editor.IncompleteEntityException;
import com.cololo.tc.tools.json.JSONException;
import com.cololo.tc.tools.json.JSONObject;
import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

public class JsonFilterTest
{
    char[] symbols = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
            's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
            'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

    @Test
    public void generate() throws IncompleteEntityException, JSONException
    {
        assertThrows(IncompleteEntityException.class, ()->new JsonFilter("asdf'sa"));
        assertThrows(IncompleteEntityException.class, ()->new JsonFilter("id == 2"));
        assertThrows(IncompleteEntityException.class, ()->new JsonFilter("id==2"));

        ArrayList<String> conditions = new ArrayList<>();
        conditions.add("=");
        conditions.add("!=");
        conditions.add("<");
        conditions.add("<=");
        conditions.add(">");
        conditions.add(">=");
        conditions.add("IN");

        String initialString = "(ID != 10 OR ID2 = \"sss\") AND SSS=\"SSasd\\\"sasd\" OR ID3!=11 OR ID3!=\"\" OR (ID4!=11 OR ID5!=11 OR ID6!=11)";
        initialString = "(ID != 10 OR ID > \"sss\") AND SSS               =             \"SSasd\\\\\\\"sasd\" OR ID3!=11 OR (ID4<=11 OR ID5 in (11,12 , 13) OR ID6 in(\"11\",\"\",\"   sa  \"))";

        for ( int i = 0; i < 50; ++i )
        {
            JSONObject object = new JSONObject();
            String key = getRandomWord();
            String cond = conditions.get((int) (Math.random() * conditions.size()));
            String values = "";

            if (cond.equals("IN"))
            {
                int wordsCount = 1 + (int) (Math.random() * 5);
                List<String> valuesArr = new ArrayList<>();
                List<String> valuesArrWithQuote = new ArrayList<>();

                boolean isString = (int)(Math.random() * 2) == 1;
                if (isString)
                {
                    for (int j = 0; j < wordsCount; j++)
                    {
                        valuesArr.add(getRandomWord());
                    }

                    valuesArr.forEach(e -> valuesArrWithQuote.add("\"" + e + "\""));
                }
                else
                {
                    for (int j = 0; j < wordsCount; j++)
                    {
                        int intVal = (int) (Math.random() * 200);
                        valuesArr.add(String.valueOf(intVal));
                        valuesArrWithQuote.add(String.valueOf(intVal));
                    }
                }

                values = valuesArrWithQuote.stream().collect(Collectors.joining(",", "(", ")"));
                if(isString) object.put(key,valuesArr.get((int) (Math.random() * valuesArr.size())));
                if(!isString) object.put(key,Integer.valueOf(valuesArr.get((int) (Math.random() * valuesArr.size()))));
            }
            else if (cond.equals("<") || cond.equals("<=") || cond.equals(">") || cond.equals(">="))
            {
                values = String.valueOf((int) (Math.random() * 6));
                object.put(key,Integer.valueOf(values));
            }
            else
            {
                values = getRandomWord();
                object.put(key,values);
                values = "\"" + values + "\"";
            }

            initialString = key + " " + cond + " " + values;

            JsonFilter filter = new JsonFilter(initialString);

            if (cond.equals("<") || cond.equals(">") || cond.equals("!=")) assertFalse( filter.isFiltered(object));
            else assertTrue( filter.isFiltered(object));
        }
    }

    private String getRandomWord()
    {
        int len = 1 + (int) (Math.random() * 10);

        StringBuilder word = new StringBuilder(len);
        for (int j = 0; j < len; ++j)
        {
            word.append(symbols[(int) ( Math.random() * symbols.length )]);
        }

        return word.toString();
    }

}