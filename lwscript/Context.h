#pragma once
#include <string_view>
#include <unordered_map>

class Context
{
public:
    Context();
    Context(Context *upContext);
    ~Context();

    void DefineVariable(std::string_view name, struct Object *value);

    void AssignVariable(std::string_view name, struct Object *value);
   struct Object *GetVariable(std::string_view name);
    Context *GetUpContext();
    void SetUpContext(Context* env);

    bool IsEqualTo(Context* env);

private:
    friend class VM;
    friend struct StructObject;

    std::unordered_map<std::string, struct Object *> m_Values;
    Context *m_UpContext;

};