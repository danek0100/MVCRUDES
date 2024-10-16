#include <utility>

#ifndef SPHINXD_RELATION_H
#define SPHINXD_RELATION_H


class Relation {
public:
    enum RelationType
    {
        NO_ACTION   = 0,    // не следует предпринимать никаких действий, но проверка целостности данных все равно будет выполнена
        RESTRICT    = 1,    // операция (удаление или обновление) будет отклонена немедленно, если существуют связанные записи
        CASCADE     = 2,    // eсли запись в родительской таблице обновляется или удаляется, соответствующие записи в дочерней таблице будут обновлены или удалены
        SET_NULL    = 3,    // eсли запись в родительской таблице удаляется или обновляется, все соответствующие записи в дочерней таблице будут обновлены так, чтобы соответствующее поле стало NULL
        SET_DEFAULT = 4	    // если запись в родительской таблице удаляется или обновляется, соответствующие записи в дочерней таблице будут обновлены так, чтобы соответствующее поле стало значением по умолчанию, определенным при создании таблицы.
    };

    static enum RelationType strToRelationType(const std::string& s) {
        if (s == "NO ACTION")   return NO_ACTION;
        if (s == "RESTRICT")    return RESTRICT;
        if (s == "CASCADE")     return CASCADE;
        if (s == "SET NULL")    return SET_NULL;
        if (s == "SET DEFAULT") return SET_DEFAULT;

        STATICTHROW(SystemException, "Incorrect relation type: " << s << "!")
        return RESTRICT;
    }

    static std::string relationTypeToStr(RelationType t) {
        if (t == NO_ACTION)   return {"NO ACTION"};
        if (t == RESTRICT)    return {"RESTRICT"};
        if (t == CASCADE)     return {"CASCADE"};
        if (t == SET_NULL)    return {"SET NULL"};
        if (t == SET_DEFAULT) return {"SET DEFAULT"};

        return {""};
    }


    Relation(const std::vector<std::string>& localFields,
             std::string  targetEntity,
             const std::vector<std::string>& targetFields,
             RelationType onDelete,
             RelationType onUpdate
             ):
            localFields(localFields),
            targetEntity(std::move(targetEntity)),
            targetFields(targetFields),
            onDelete(onDelete),
            onUpdate(onUpdate) {}

    Relation() = default;

    std::vector<std::string> getLocalFields() const { return localFields; }
    std::string getTargetEntity() const { return targetEntity; }
    std::vector<std::string> getTargetFields() const { return targetFields; }
    RelationType getOnDelete() const { return onDelete; }
    RelationType getOnUpdate() const { return onUpdate; }

private:
    std::vector<std::string> localFields;
    std::string targetEntity;
    std::vector<std::string> targetFields;
    RelationType onDelete;
    RelationType onUpdate;
};

#endif //SPHINXD_RELATION_H
