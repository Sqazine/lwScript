#pragma once
#include "Config.h"
#include "Chunk.h"
#include "Object.h"
#include "Library.h"
namespace lws
{
    struct CallFrame
    {
        ClosureObject *closure = nullptr;
        uint8_t *ip = nullptr;
        Value *slots = nullptr;
    };

    class VM
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
        Value Peek(int32_t distance);

        template <class T, typename... Args>
        T *CreateObject(Args &&...params);

        template <class T>
        void FreeObject(T *object);

        UpValueObject *CaptureUpValue(Value *location);
        void ClosedUpValues(Value *end);

        UpValueObject *mOpenUpValues;

        LibraryManager mLibraryManager;

        static Value sNullValue;

        Value mGlobalVariables[GLOBAL_VARIABLE_MAX];

        Value *mStackTop;
        Value mValueStack[STACK_MAX];

        CallFrame mFrames[STACK_MAX];
        int32_t mFrameCount;

        Object *mObjectChain;

        size_t mBytesAllocated;
    };

    template <class T, typename... Args>
    inline T *VM::CreateObject(Args &&...params)
    {
        mBytesAllocated += sizeof(T);

        T *object = new T(std::forward<Args>(params)...);
        object->next = mObjectChain;
        object->marked = false;
        mObjectChain = object;

        return object;
    }

    template <class T>
    inline void VM::FreeObject(T *object)
    {
        mBytesAllocated -= sizeof(T);
        delete object;
    }
}