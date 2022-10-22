#pragma once
#include "Config.h"
#include "Chunk.h"
#include "Object.h"
#include "Library.h"
namespace lws
{

    struct CallFrame
    {
        FunctionObject *function;
        uint8_t *ip;
        Value *slots;
    };

    class VM
    {
    public:
        VM();
        ~VM();

        void ResetStatus();

        void Run(FunctionObject *mainFunc);

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

        void BindClassFunction(ClassObject* klass,const std::wstring& funcName);

        LibraryManager mLibraryManager;

        Value mGlobalVariables[GLOBAL_VARIABLE_MAX];

        Value *mStackTop;
        Value mValueStack[STACK_MAX];

        CallFrame mFrames[STACK_MAX];
        int32_t mFrameCount;

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