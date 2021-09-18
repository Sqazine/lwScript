#pragma once
#include <unordered_map>
#include <iostream>
#include <cstdio>
#include "Object.h"
namespace lwScript
{
    class Library
    {
    public:
        ~Library() {}

        static void AddNativeFunctionObject(NativeFunctionObject *fn)
        {
            for (const auto &f : m_Functions)
                if (f->name == fn->name)
                    std::cout << "Redefinite native function:" << fn->name;
            m_Functions.emplace_back(fn);
        }

        static NativeFunctionObject *GetNativeFunctionObject(std::string_view fnName)
        {
            for (const auto &fnObj : m_Functions)
                if (fnObj->name == fnName)
                    return fnObj;
            std::cout << "No function:" << fnName;
            exit(1);
        }

    private:
        Library() {}
        static std::vector<NativeFunctionObject *> m_Functions;
    };

    std::vector<NativeFunctionObject *> Library::m_Functions =
        {
            new NativeFunctionObject("print", [](std::vector<Object *> args) -> Object *
                                     {
                                         if (args.empty())
                                             return nilObject;

                                         if (args[0]->Type() != ObjectType::STR)
                                         {
                                             std::cout << "Invalid argument:The first argument of native print fn must be string type." << std::endl;
                                             exit(1);
                                         }
                                         if (args.size() == 1)
                                             std::cout << args[0]->Stringify();
                                         else//formatting output
                                         {
                                             std::string content = TO_STR_OBJ(args[0])->value;

                                             int32_t pos = content.find("{}");
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
                                         return nilObject;
                                     }),
    };
}