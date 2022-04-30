#pragma once
#include <array>
#include <cstdint>
#include <stack>
#include <functional>
#include <unordered_map>
#include <string>
#include <string_view>
#include "Frame.h"
#include "Object.h"
#include "Utils.h"
#include "Context.h"
namespace lws
{
#define STACK_MAX 1024
#define GC_OBJECT_COUNT_THRESHOLD 256

	class VM
	{
	public:
		VM();
		~VM();

		void ResetStatus();
		Object *Execute(Frame *frame);

	private:
		void PreAssemble(Frame *frame);
		Object *ExecuteOpCode(Frame *frame);

		friend class Linrary;
		friend class DataStructure;
		friend class Memory;
		friend class IO;

		RealNumObject *CreateRealNumObject(double value = 0.0);
		IntNumObject *CreateIntNumObject(int64_t value = 0);
		StrObject *CreateStrObject(std::wstring_view value = L"");
		BoolObject *CreateBoolObject(bool value = false);
		NullObject *CreateNullObject();
		ArrayObject *CreateArrayObject(const std::vector<Object *> &elements = {});
		TableObject *CreateTableObject(const std::unordered_map<Object *, Object *> &elements = {});
		FieldObject *CreateFieldObject(std::wstring_view name, const std::unordered_map<std::wstring, ObjectDesc> &members, const std::vector<std::pair<std::wstring, FieldObject *>> &containedFields = {});
		LambdaObject *CreateLambdaObject(int64_t frameIdx);
		RefObject *CreateRefObject(std::wstring_view name, Object *index);
		RefObject *CreateRefObject(std::wstring_view address);

		Object *CopyObject(Object *srcObj);

		bool IsPassedByValueObject(Object* obj);

		void Gc();

		std::function<Object *(std::vector<Object *>)> GetNativeFunction(std::wstring_view fnName);
		bool HasNativeFunction(std::wstring_view name);

		void PushObject(Object *object);
		Object *PopObject();

		void PushFrame(Frame *frame);
		Frame *PopFrame();
		bool IsFrameStackEmpty();

		uint64_t sp;
		std::array<Object *, STACK_MAX> mObjectStack;

		uint64_t fp;
		std::array<Frame *, STACK_MAX> mFrameStack;

		Object *firstObject;
		int curObjCount;
		int maxObjCount;

		Context *mContext;
	};
}