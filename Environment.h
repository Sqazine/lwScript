#pragma once
#include <string_view>
#include "Object.h"

class Environment
{
public:
    Environment(class VM *vm);
    Environment(class VM *vm, Environment *upEnvironment);
    ~Environment();

    void DefineVariable(std::string_view name, Object *value);

    void AssignVariable(std::string_view name, Object *value);
    Object *GetVariable(std::string_view name);
    Environment *GetUpEnvironment();

private:
    std::unordered_map<std::string, Object *> m_Values;
    Environment *m_UpEnvironment;

    class VM *m_VMHandle;
};