#pragma once
#include <unordered_map>
#include <functional>
#include "Value.h"
#include "Object.h"
namespace lws
{
    class StdLibraries
    {
    public:
        static StdLibraries &Instance() noexcept;

    private:
        StdLibraries();
        ~StdLibraries();

        StdLibraries(const StdLibraries &) = delete;
        StdLibraries(StdLibraries &&) = delete;
        StdLibraries &operator=(const StdLibraries &) = delete;
        StdLibraries &operator=(StdLibraries &&) = delete;

        friend class VM;
        friend class Compiler;
        std::vector<std::wstring> mLibraryMap;
        std::vector<ClassObject *> mLibraries;
    };
}