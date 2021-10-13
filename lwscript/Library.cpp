#include "Library.h"
#include "VM.h"
namespace lws
{

    Library::Library(VM *vm)
        : m_VMHandle(vm)
    {
    }
    Library::~Library()
    {
        std::unordered_map<std::string, std::function<Object *(std::vector<Object *>)>>().swap(m_NativeFunctions);
    }

    void Library::AddNativeFunction(std::string_view name, std::function<Object *(std::vector<Object *>)> fn)
    {
        auto iter = m_NativeFunctions.find(name.data());
        if (iter != m_NativeFunctions.end())
            Assert(std::string("Already exists native function:") + name.data());
        m_NativeFunctions[name.data()] = fn;
    }
    std::function<Object *(std::vector<Object *>)> Library::GetNativeFunction(std::string_view fnName)
    {
        auto iter = m_NativeFunctions.find(fnName.data());
        if (iter != m_NativeFunctions.end())
            return iter->second;
        Assert(std::string("No native function:") + fnName.data());

        return nullptr;
    }
    bool Library::HasNativeFunction(std::string_view name)
    {
        auto iter = m_NativeFunctions.find(name.data());
        if (iter != m_NativeFunctions.end())
            return true;
        return false;
    }

    IO::IO(VM *vm)
        : Library(vm)
    {
        m_NativeFunctions["println"] = [this](std::vector<Object *> args) -> Object *
        {
            if (args.empty())
                return nullptr;

            if (args[0]->Type() != ObjectType::STR)
                Assert("Invalid argument:The first argument of native print function must be string type.");
            if (args.size() == 1)
                std::cout << args[0]->Stringify() << std::endl;
            else //formatting output
            {
                std::string content = TO_STR_OBJ(args[0])->value;

                int32_t pos = (int32_t)content.find("{}");
                int32_t argpos = 1;
                while (pos != std::string::npos)
                {
                    if (argpos < args.size())
                        content.replace(pos, 2, args[argpos++]->Stringify());
                    else
                        content.replace(pos, 2, "nil");
                    pos = content.find("{}");
                }

                std::cout << content << std::endl;
            }
            return nullptr;
        };
    }

    DataStructure::DataStructure(VM *vm)
        : Library(vm)
    {
        m_NativeFunctions["sizeof"] = [this](std::vector<Object *> args) -> Object *
        {
            if (args.empty() || args.size() > 1)
                Assert("[Native function 'sizeof']:Expect a argument.");

            if (IS_ARRAY_OBJ(args[0]))
                return m_VMHandle->CreateIntegerObject(TO_ARRAY_OBJ(args[0])->elements.size());
            else if(IS_TABLE_OBJ(args[0]))
                return m_VMHandle->CreateIntegerObject(TO_TABLE_OBJ(args[0])->elements.size());
            else if(IS_STR_OBJ(args[0]))
                return m_VMHandle->CreateIntegerObject(TO_STR_OBJ(args[0])->value.size());
            else 
                Assert("[Native function 'sizeof']:Expect a array,table ot string argument.");
            return nullptr;
        };
    }
}