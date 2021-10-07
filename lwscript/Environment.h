#pragma once
#include <string_view>
#include <unordered_map>

class Environment
{
public:
    Environment();
    Environment(Environment *upEnvironment);
    ~Environment();

    void DefineVariable(std::string_view name, struct Object *value);

    void AssignVariable(std::string_view name, struct Object *value);
   struct Object *GetVariable(std::string_view name);
    Environment *GetUpEnvironment();
    void SetUpEnvironment(Environment* env);

private:
    friend class VM;
    friend struct StructObject;

    std::unordered_map<std::string, struct Object *> m_Values;
    Environment *m_UpEnvironment;

};