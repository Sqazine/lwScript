#include "Allocator.h"
#include "VM.h"
namespace lwscript
{

    Allocator::Allocator(VM* vm)
        : mVM(vm), mObjectChain(nullptr)
    {
        ResetStatus();
    }
    Allocator::~Allocator()
    {
        FreeObjects();
    }

    void Allocator::ResetStatus()
    {
        if (!mObjectChain)
            FreeObjects();

        mBytesAllocated = 0;
        mNextGCByteSize = 256;
        mObjectChain = nullptr;
    }

    void Allocator::FreeObjects()
    {
        auto bytes = mBytesAllocated;
        Object *object = mObjectChain;
        while (object != nullptr)
        {
            Object *next = object->next;
            mBytesAllocated -= sizeof(object);
            SAFE_DELETE(object);
            object = next;
        }

#ifdef GC_DEBUG
        std::wcout << "collected " << bytes - mBytesAllocated << " bytes (from " << bytes << " to " << mBytesAllocated << ") next gc bytes " << mNextGCByteSize << std::endl;
#endif
    }

    void Allocator::RegisterToGCRecordChain(const Value &value)
    {
        if (IS_OBJECT_VALUE(value) && value.object->next == nullptr) // check is null to judge if is a unique object
        {
            size_t objBytes = sizeof(value.object);
            mBytesAllocated += objBytes;
#ifdef GC_STRESS
            GC();
#endif
            if (mBytesAllocated > mNextGCByteSize)
                GC();
            value.object->marked = false;
            value.object->next = mObjectChain;
            mObjectChain = value.object;
#ifdef GC_DEBUG
            std::cout << (void *)value.object << " has been add to gc record chain " << objBytes << " for " << value.object->type << std::endl;
#endif
        }
    }
    void Allocator::GC()
    {
#ifdef GC_DEBUG
        std::wcout << "begin gc" << std::endl;
        size_t bytes = mBytesAllocated;
#endif
        MarkRootObjects();
        MarkGrayObjects();
        Sweep();
        mNextGCByteSize = mBytesAllocated * GC_HEAP_GROW_FACTOR;
#ifdef GC_DEBUG
        std::wcout << "end gc" << std::endl;
        std::wcout << "    collected " << bytes - mBytesAllocated << " bytes (from " << bytes << " to " << mBytesAllocated << ") next gc bytes " << mNextGCByteSize << std::endl;
#endif
    }
    void Allocator::MarkRootObjects()
    {
        for (Value *slot = mVM->mValueStack; slot < mVM-> mStackTop; ++slot)
            slot->Mark(this);
        for (int32_t i = 0; i < mVM->mFrameCount; ++i)
            mVM->mFrames[i].closure->Mark(this);
        for (UpValueObject *upvalue = mVM->mOpenUpValues; upvalue != nullptr; upvalue = upvalue->nextUpValue)
            upvalue->Mark(this);

        for (int32_t i = 0; i < GLOBAL_VARIABLE_MAX; ++i)
            if (mVM->mGlobalVariables[i] != VM::sNullValue)
                mVM->mGlobalVariables[i].Mark(this);
    }
    void Allocator::MarkGrayObjects()
    {
        while (mGrayObjects.size() > 0)
        {
            auto object = mGrayObjects.back();
            mGrayObjects.pop_back();
            object->Blacken(this);
        }
    }
    void Allocator::Sweep()
    {
        Object *previous = nullptr;
        Object *object = mObjectChain;
        while (object)
        {
            if (object->marked)
            {
                object->UnMark();
                previous = object;
                object = object->next;
            }
            else
            {
                Object *unreached = object;
                object = object->next;
                if (previous != nullptr)
                    previous->next = object;
                else
                    mObjectChain = object;

                FreeObject(unreached);
            }
        }
    }
}