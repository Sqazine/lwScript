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

		template<class T,typename ...Args>
		T* CreateObject(Args&& ...params);


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

		Context mContext;
	};

	template<class T,typename ...Args>
	inline	T* VM::CreateObject(Args&& ...params)
	{
		T* object= new T(std::forward<Args>(params)...);
		AddToObjectList(object);
		return object;
	}
}