#pragma once
#include <array>
#include <cstdint>
#include <stack>
#include <functional>
#include <unordered_map>
#include <string>
#include <string_view>
#include <map>
#include "Frame.h"
#include "Object.h"
#include "Utils.h"
#include "Context.h"
#include "Config.h"
namespace lws
{

	struct ExecuteRecorder
	{
		void Dump();

		struct record
		{
			uint64_t callTimes=0;
			uint64_t executeTime=0;
		};

		std::unordered_map<OpCode,record> mOpCodeCallTimes;
	};

#define STACK_INCREMENT_RATE 2
#define VALUE_STACK_MAX 8192
#define FRAME_STACK_MAX 256
#define GC_OBJECT_COUNT_THRESHOLD 4096

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
		ClassObject *CreateClassObject(std::wstring_view name, const std::unordered_map<std::wstring, ValueDesc> &members, const std::vector<std::pair<std::wstring, ClassObject *>> &parentClasses = {});
		LambdaObject *CreateLambdaObject(int64_t frameIdx);
		RefObject *CreateRefObject(std::wstring_view name, Value index);
		RefObject *CreateRefObject(std::wstring_view address);

		std::function<Value(std::vector<Value>)> GetNativeFunction(std::wstring_view fnName);
		bool HasNativeFunction(std::wstring_view name);

		void PushValue(Value object);
		Value PopValue();

		void PushFrame(Frame *frame);
		Frame *PopFrame();
		bool IsFrameStackEmpty();

		void Gc();
		void AddToObjectList(Object* object);

		uint64_t sp;
		uint64_t curValueStackSize;
		std::vector<Value> mValueStack;

		uint64_t fp;
		uint64_t curFrameStackSize;
		std::vector<Frame *> mFrameStack;

		std::unordered_map<std::wstring,Value> mStringTable;

		Object *objectListsHead;
		int curObjCount;
		int maxObjCount;

		Context *mContext;

		#ifdef NEED_TO_RECORD_OPCODE_EXECUTE
			ExecuteRecorder mRecorder;
		#endif
	};
}