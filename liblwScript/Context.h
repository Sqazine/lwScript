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

        void DefineVariableByName(std::wstring_view name, ObjectDescType objDescType, struct Object *value);
        void DefineVariableByName(std::wstring_view name, const ObjectDesc& objectDesc);
        void AssignVariableByName(std::wstring_view name, struct Object *value);
        struct Object *GetVariableByName(std::wstring_view name);

		void AssignVariableByAddress(std::wstring_view address, struct Object* value);
		struct Object* GetVariableByAddress(std::wstring_view address);

        Context *GetUpContext();
        void SetUpContext(Context *env);

        Context *GetRoot();

    private:
        friend class VM;
        friend struct FieldObject;

        std::unordered_map<std::wstring, ObjectDesc> mValues;
        Context *mUpContext;
    };
}