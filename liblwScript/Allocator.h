#pragma once
#include <vector>
#include "Object.h"
#include "Value.h"
namespace lwscript
{

    class LWSCRIPT_API Allocator
    {
    public:
        Allocator(class VM *vm);
        ~Allocator();

        void ResetStatus();

        template <class T, typename... Args>
        T *CreateObject(Args &&...params);

        void RegisterToGCRecordChain(const Value &value);

    private:
        template <class T>
        void FreeObject(T *object);
        void FreeObjects();
        void GC();

        void MarkRootObjects();
        void MarkGrayObjects();
        void Sweep();

        class VM *mVM;

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
        size_t objBytes = sizeof(*object);
        mBytesAllocated += objBytes;
#ifdef GC_STRESS
        GC();
#endif
        if (mBytesAllocated > mNextGCByteSize)
            GC();

        object->next = mObjectChain;
        object->marked = false;
        mObjectChain = object;
#ifdef GC_DEBUG
        std::cout << (void *)object << " has been add to gc record chain " << objBytes << " for " << object->type << std::endl;
#endif

        return object;
    }

    template <class T>
    inline void Allocator::FreeObject(T *object)
    {
#ifdef GC_DEBUG
        Println(L"delete object(0x{})",(void*)object);
#endif
        mBytesAllocated -= sizeof(object);
        SAFE_DELETE(object);
    }
}