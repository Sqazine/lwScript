#pragma once
#include <vector>
#include "Object.h"
#include "Value.h"
namespace lwscript
{

    class LWSCRIPT_API Allocator
    {
    public:
        Allocator(class VM* vm);
        ~Allocator();

        void ResetStatus();

        template <class T, typename... Args>
        T *CreateObject(Args &&...params);

        template <class T>
        void FreeObject(T *object);
        void FreeObjects();

        void RegisterToGCRecordChain(const Value &value);
        void MarkRootObjects();
        void MarkGrayObjects();
        void Sweep();

    private:
        void GC();

        class VM* mVM;

        friend struct Object;

        Object *mObjectChain;
        std::vector<Object *> mGrayObjects;
        size_t mBytesAllocated;
        size_t mNextGCByteSize;
    };

    template <class T, typename... Args>
    inline T *Allocator::CreateObject(Args &&...params)
    {
        T *object = new T(std::forward<Args>(params)...);
        object->next = mObjectChain;
        object->marked = false;
        mObjectChain = object;

        mBytesAllocated += sizeof(object);

        return object;
    }

    template <class T>
    inline void Allocator::FreeObject(T *object)
    {
#ifdef GC_DEBUG
        std::wcout << L"delete object(0x" << (void *)object << L")" << std::endl;
#endif
        mBytesAllocated -= sizeof(object);
        delete object;
    }
}