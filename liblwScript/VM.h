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
		Value Execute(Frame *frame);

	private:
		void PreAssemble(Frame *frame);
		Value ExecuteOpCode(Frame *frame);

		friend class Linrary;
		friend class DataStructure;
		friend class Memory;
		friend class IO;

		StrObject *CreateStrObject(std::wstring_view value);
		ArrayObject *CreateArrayObject(const std::vector<Value> &elements = {});
		TableObject *CreateTableObject(const ValueUnorderedMap &elements = {});
		FieldObject *CreateFieldObject(std::wstring_view name, const std::unordered_map<std::wstring, ValueDesc> &members, const std::vector<std::pair<std::wstring, FieldObject *>> &containedFields = {});
		LambdaObject *CreateLambdaObject(int64_t frameIdx);
		RefObject *CreateRefObject(std::wstring_view name, Value index);
		RefObject *CreateRefObject(std::wstring_view address);

		void Gc();

		std::function<Value(std::vector<Value>)> GetNativeFunction(std::wstring_view fnName);
		bool HasNativeFunction(std::wstring_view name);

		void PushValue(Value object);
		Value PopValue();

		void PushFrame(Frame *frame);
		Frame *PopFrame();
		bool IsFrameStackEmpty();

		uint64_t sp;
		std::array<Value, STACK_MAX> mValueStack;

		uint64_t fp;
		std::array<Frame *, STACK_MAX> mFrameStack;

		Object *firstObject;
		int curObjCount;
		int maxObjCount;

		Context *mContext;
	};
}