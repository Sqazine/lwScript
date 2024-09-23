#pragma once
#include <unordered_map>
#include <functional>
#include "Value.h"
#include "Object.h"
namespace lwscript
{
    class LWSCRIPT_API LibraryManager
    {
    public:
        static LibraryManager &Instance() noexcept;

        void RegisterLibrary(ClassObject *libraryClass);

        const std::vector<ClassObject *> &GetLibraries() const;

    private:
        LibraryManager();
        ~LibraryManager() = default;

        LibraryManager(const LibraryManager &) = delete;
        LibraryManager(LibraryManager &&) = delete;
        LibraryManager &operator=(const LibraryManager &) = delete;
        LibraryManager &operator=(LibraryManager &&) = delete;

        std::vector<ClassObject *> mLibraries;
    };
}