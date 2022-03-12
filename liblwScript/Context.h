#pragma once
#include <string_view>
#include <unordered_map>
namespace lws
{
    class Context
    {
    public:
        Context();
        Context(Context *upContext);
        ~Context();

        void DefineVariableByName(std::string_view name, struct Object *value);

        void AssignVariableByName(std::string_view name, struct Object *value);
        struct Object *GetVariableByName(std::string_view name);

        Context *GetUpContext();
        void SetUpContext(Context *env);

        const std::unordered_map<std::string, struct Object*>& GetValues() const;
    private:
        friend class VM;
        friend struct FieldObject;

        std::unordered_map<std::string, struct Object *> mValues;
        Context *mUpContext;
    };
}