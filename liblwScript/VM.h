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

        bool IsFalsey(const Value& v);

        void Push(const Value &value);
        Value Pop();
        Value Peek(int32_t distance);

        template <class T, typename... Args>
        T *CreateObject(Args &&...params);

        template <class T>
        void FreeObject(T *object);

        Value mGlobalVariables[GLOBAL_VARIABLE_MAX];

        Value *mStackTop;
        Value mValueStack[STACK_MAX];

        Chunk mChunk;

        Object *objectChain;

        friend struct Object;

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