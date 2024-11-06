#pragma once
#include <iostream>
#include <vector>
#include "Config.h"
#include "Chunk.h"
#include "Object.h"
#include "LibraryManager.h"
namespace lwscript
{
    class LWSCRIPT_API VM
    {
    public:
        VM() = default;
        ~VM() = default;

        std::vector<Value> Run(FunctionObject *mainFunc);

    private:
        void Execute();

        bool IsFalsey(const Value &v);
    };
}