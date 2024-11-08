#pragma once
#include <unordered_map>
#include <functional>
#include "Value.h"
#include "Object.h"
#include "Config.h"
namespace lwscript
{
    class LWSCRIPT_API LibraryManager
    {
        NON_COPYABLE(LibraryManager)
    public:
        SINGLETON_DECL(LibraryManager)

        void RegisterLibrary(ClassObject *libraryClass);

        const std::vector<ClassObject *> &GetLibraries() const;

    private:
        LibraryManager();
        ~LibraryManager() = default;

        std::vector<ClassObject *> mLibraries;
    };
}