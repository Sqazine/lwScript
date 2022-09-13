#pragma once
#include "Config.h"
#include "Chunk.h"
namespace lws
{
    class VM
    {
    public:
        VM();
        ~VM();

        void ResetStatus();

        void Run(const Chunk &chunk);

    private:
        void Execute();

        void Push(const Value &value);
        Value Pop();

        Value mGlobalVariables[GLOBAL_VARIABLE_MAX];

        Value *mStackTop;
        Value mValueStack[STACK_MAX];


        Chunk mChunk;
    };
}