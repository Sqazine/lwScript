#pragma once
#include "Config.h"
#include "Chunk.h"
#include "Object.h"
#include "Library.h"
namespace lws
{

    struct CallFrame
    {
        FunctionObject *function=nullptr;
        uint8_t *ip=nullptr;
        Value *slots=nullptr;
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

        LibraryManager mLibraryManager;

        static Value sNullValue;

        Value mGlobalVariables[GLOBAL_VARIABLE_MAX];

        Value *mStackTop;
        Value mValueStack[STACK_MAX];

        CallFrame mFrames[STACK_MAX];
        int32_t mFrameCount;

        Object *objectChain;

        size_t bytesAllocated;
    };

    template <class T, typename... Args>
    inline T *VM::CreateObject(Args &&...params)
    {
        bytesAllocated += sizeof(T);

        T *object = new T(std::forward<Args>(params)...);
        object->next = objectChain;
        object->marked = false;
        objectChain = object;

        return object;
    }

    template <class T>
    inline void VM::FreeObject(T *object)
    {
        bytesAllocated -= sizeof(T);
        delete object;
    }
}