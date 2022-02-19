#pragma once
#include <unordered_map>
#include <functional>
#include "Object.h"
namespace lws
{
    class Library
    {
    public:
        Library(class VM *vm);
        ~Library();

        virtual void AddNativeFunction(std::string_view name, std::function<Object *(std::vector<Object *>)> fn);
        virtual std::function<Object *(std::vector<Object *>)> GetNativeFunction(std::string_view fnName);
        virtual bool HasNativeFunction(std::string_view name);

    protected:
        std::unordered_map<std::string, std::function<Object *(std::vector<Object *>)>> m_NativeFunctions;
        class VM *m_VMHandle;
    };
  
    class IO:public Library
    {
        public:
        IO(class VM* vm);
    };

    class DataStructure:public Library
    {
        public:
        DataStructure(class VM* vm);
    };

    class LibraryManager
    {
    public:
         static void RegisterLibrary(std::string_view name,Library* lib);
         static bool HasNativeFunction(std::string_view name);
    private:
        LibraryManager() {}
        friend class VM;
        static std::unordered_map<std::string,Library*> m_Libraries;
    };
}