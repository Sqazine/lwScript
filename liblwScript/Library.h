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

        virtual void AddNativeFunction(std::wstring_view name, std::function<Object *(std::vector<Object *>)> fn);
        virtual std::function<Object *(std::vector<Object *>)> GetNativeFunction(std::wstring_view fnName);
        virtual bool HasNativeFunction(std::wstring_view name);

    protected:
        std::unordered_map<std::wstring, std::function<Object *(std::vector<Object *>)>> mNativeFunctions;
        class VM *mVMHandle;
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

    class Memory : public Library
    {
    public:
        Memory(class VM *vm);
    };

    class LibraryManager
    {
    public:
         static void RegisterLibrary(std::wstring_view name,Library* lib);
         static bool HasNativeFunction(std::wstring_view name);
    private:
        LibraryManager() {}
        friend class VM;
        static std::unordered_map<std::wstring,Library*> mLibraries;
    };
}