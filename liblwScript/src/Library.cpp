#include "Library.h"
#include "Utils.h"

#include <stdio.h>
#include <ctime>
#include <iostream>
namespace lwscript
{
    LibraryManager &LibraryManager::Instance() noexcept
    {
        static LibraryManager instance;
        return instance;
    }

    void LibraryManager::RegisterLibrary(ClassObject *libraryClass)
    {
        for (const auto &lib : mLibraries)
        {
            if (lib->name == libraryClass->name)
                Hint::Error(nullptr, L"Conflict library name {}", libraryClass->name);
        }

        mLibraries.emplace_back(libraryClass);
    }

    const std::vector<ClassObject *> &LibraryManager::GetLibraries() const
    {
        return mLibraries;
    }

    LibraryManager::LibraryManager()
    {
#include "StdLibraries/IO.inl"
#include "StdLibraries/DS.inl"
#include "StdLibraries/Mem.inl"
#include "StdLibraries/Time.inl"
    }
}