#pragma once
#include <iostream>
#include <vector>
#include "Config.h"
#include "Chunk.h"
#include "Object.h"
#include "Library.h"
#include "Allocator.h"
namespace lwscript
{
    struct CallFrame
    {
        ClosureObject *closure = nullptr;
        uint8_t *ip = nullptr;
        Value *slots = nullptr;
#ifdef USE_FUNCTION_CACHE
        std::vector<Value> arguments;
#endif
    };


    class LWSCRIPT_API VM
    {
    public:
        VM();
        ~VM();

        void ResetStatus();

        std::vector<Value> Run(FunctionObject *mainFunc);

    private:
        void Execute();

        bool IsFalsey(const Value &v);

        void Push(const Value &value);
        Value Pop();
        Value Peek(int32_t distance=0);

        UpValueObject *CaptureUpValue(Value *location);
        void ClosedUpValues(Value *end);

        UpValueObject *mOpenUpValues;

        friend class Allocator;

        Value mGlobalVariables[GLOBAL_VARIABLE_MAX];

        Value *mStackTop;
        Value mValueStack[STACK_MAX];

        CallFrame mFrames[STACK_MAX];
        int32_t mFrameCount;

        Allocator* mAllocator;  
    };
}