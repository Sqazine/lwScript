#include "Environment.h"
#include "Utils.h"
#include "VM.h"

Environment::Environment(VM *vm) : m_VMHandle(vm), m_UpEnvironment(nullptr) {}
Environment::Environment(VM *vm, Environment *upEnvironment) : m_VMHandle(vm), m_UpEnvironment(upEnvironment) {}
Environment::~Environment()
{
    for (const auto &[key, value] : m_Values)
        value->UnMark();
    m_VMHandle->Gc();
}

void Environment::DefineVariable(std::string_view name, Object *value)
{
    value->Mark(); //保存到环境中,先进行标记,避免被回收
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
    {
        value->Mark(); //保存到环境中,先进行标记,避免被回收
        m_Values[name.data()] = value;
    }
    else if (m_UpEnvironment != nullptr)
        m_UpEnvironment->AssignVariable(name, value);
    else
        Assert("Undefine variable:" + std::string(name) + " in current context");
}

Object *Environment::GetVariable(std::string_view name)
{
    auto iter = m_Values.find(name.data());

    if (iter != m_Values.end())
    {
        iter->second->UnMark(); //退出环境,解除标记
        return iter->second;
    }

    if (m_UpEnvironment != nullptr)
        return m_UpEnvironment->GetVariable(name);

    return m_VMHandle->CreateNilObject();
}

Environment *Environment::GetUpEnvironment()
{
    return m_UpEnvironment;
}