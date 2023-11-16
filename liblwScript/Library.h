#pragma once
#include <unordered_map>
#include <functional>
#include "Value.h"
#include "Object.h"
namespace lws
{
    class LWSCRIPT_API LibraryManager
    {
    public:
        static LibraryManager &Instance() noexcept;

    private:
        LibraryManager();
        ~LibraryManager();

        LibraryManager(const LibraryManager &) = delete;
        LibraryManager(LibraryManager &&) = delete;
        LibraryManager &operator=(const LibraryManager &) = delete;
        LibraryManager &operator=(LibraryManager &&) = delete;

        friend class VM;
        friend class Compiler;
        std::vector<std::wstring> mStdLibraryMap;
        std::vector<ClassObject *> mStdLibraries;

        std::vector<std::wstring> mLibraryMap;
        std::vector<ClassObject *> mLibraries;
    };
}