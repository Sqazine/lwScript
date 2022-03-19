#pragma once
#include <string_view>
#include <unordered_map>
namespace lws
{
    enum class ObjectDescType
    {
        CONST,
        VARIABLE
    };

    struct ObjectDesc
    {
        ObjectDescType type;
        struct Object *object;
    };

    class Context
    {
    public:
        Context();
        Context(Context *upContext);
        ~Context();

        void DefineVariableByName(std::string_view name, ObjectDescType objDescType, struct Object *value);
        void DefineVariableByName(std::string_view name, const ObjectDesc& objectDesc);
        void AssignVariableByName(std::string_view name, struct Object *value);
        struct Object *GetVariableByName(std::string_view name);

        Context *GetUpContext();
        void SetUpContext(Context *env);
    private:
        friend class VM;
        friend struct FieldObject;

        std::unordered_map<std::string, ObjectDesc> mValues;
        Context *mUpContext;
    };
}