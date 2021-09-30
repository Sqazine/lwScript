#include "Environment.h"
#include "Utils.h"
#include "VM.h"
#include "Object.h"

Environment::Environment(VM *vm) : m_VMHandle(vm), m_UpEnvironment(nullptr) {}
Environment::Environment(VM *vm, Environment *upEnvironment) : m_VMHandle(vm), m_UpEnvironment(upEnvironment) {}
Environment::~Environment()
{
}

void Environment::DefineVariable(std::string_view name, Object *value)
{
    auto iter = m_Values.find(name.data());
    if (iter != m_Values.end())
        Assert("Redefined variable:" + std::string(name) + " in current context.");
    else
        m_Values[name.data()] = value;
}

void Environment::AssignVariable(std::string_view name, Object *value)
{
    auto iter = m_Values.find(name.data());
    if (iter != m_Values.end())
        m_Values[name.data()] = value;
    else if (m_UpEnvironment != nullptr)
        m_UpEnvironment->AssignVariable(name, value);
    else
        Assert("Undefine variable:" + std::string(name) + " in current context");
}

Object *Environment::GetVariable(std::string_view name)
{
    auto iter = m_Values.find(name.data());

    if (iter != m_Values.end())
        return iter->second;

    if (m_UpEnvironment != nullptr)
        return m_UpEnvironment->GetVariable(name);

    return nullptr;
}

Environment *Environment::GetUpEnvironment()
{
    return m_UpEnvironment;
}

void Environment::SetUpEnvironment(Environment* env)
{
    m_UpEnvironment = env;
}
