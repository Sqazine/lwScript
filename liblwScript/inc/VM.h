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

#ifdef USE_FUNCTION_CACHE
    class FunctionCache
    {
    public:
        void Set(const std::wstring& name, const std::vector<Value>& arguments, const std::vector<Value>& result);
		bool Get(const std::wstring& name, const std::vector<Value>& arguments, std::vector<Value>& result) const;
#ifdef PRINT_FUNCTION_CACHE
        void Print();
#endif
    private:
        std::unordered_map<std::wstring, ValueVecUnorderedMap> mCaches;
    };
#endif

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

        static Value sNullValue;

        friend class Allocator;

        Value mGlobalVariables[GLOBAL_VARIABLE_MAX];

        Value *mStackTop;
        Value mValueStack[STACK_MAX];

        CallFrame mFrames[STACK_MAX];
        int32_t mFrameCount;

        Allocator* mAllocator;

#ifdef USE_FUNCTION_CACHE
        FunctionCache mFunctionCache;
#endif       
    };
}