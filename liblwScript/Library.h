#pragma once
#include <unordered_map>
#include <functional>
#include "Value.h"
#include "Object.h"
namespace lws
{
    static std::vector<std::wstring> gLibraryMap =
        {
            L"io",
            L"ds",
            L"mem",
            L"time"};
    class LibraryManager
    {
    public:
        LibraryManager();
        ~LibraryManager();

        void AddLibrary(ClassObject *libClass);

    protected:
        friend class VM;
        std::vector<ClassObject *> mLibraries;
    };
}