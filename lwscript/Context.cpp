#include "Context.h"
#include "Utils.h"
#include "VM.h"
#include "Object.h"
namespace lws
{
    Context::Context() : m_UpContext(nullptr) {}
    Context::Context(Context *upContext) : m_UpContext(upContext) {}
    Context::~Context() {}

    void Context::DefineVariable(std::string_view name, Object *value)
    {
        auto iter = m_Values.find(name.data());
        if (iter != m_Values.end())
            Assert("Redefined variable:" + std::string(name) + " in current context.");
        else
            m_Values[name.data()] = value;
    }

    void Context::AssignVariable(std::string_view name, Object *value)
    {
        auto iter = m_Values.find(name.data());
        if (iter != m_Values.end())
            m_Values[name.data()] = value;
        else if (m_UpContext != nullptr)
            m_UpContext->AssignVariable(name, value);
        else
            Assert("Undefine variable:" + std::string(name) + " in current context");
    }

    Object *Context::GetVariable(std::string_view name)
    {
        auto iter = m_Values.find(name.data());

        if (iter != m_Values.end())
            return iter->second;

        if (m_UpContext != nullptr)
            return m_UpContext->GetVariable(name);

        return nullptr;
    }

    Context *Context::GetUpContext()
    {
        return m_UpContext;
    }

    void Context::SetUpContext(Context *env)
    {
        m_UpContext = env;
    }
    const std::unordered_map<std::string, struct Object*>& Context::GetValues() const
    {
        return m_Values;
    }
}