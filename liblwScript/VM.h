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
#define INIT_OBJ_NUM_MAX 8192

	class VM
	{
	public:
		VM();
		~VM();

		void ResetStatus();
		Object *Execute(Frame *frame);
	private:
		friend class Linrary;
		friend class DataStructure;
		friend class Memory;
		friend class IO;

		RealNumObject *CreateRealNumObject(double value = 0.0);
		IntNumObject *CreateIntNumObject(int64_t value = 0);
		StrObject *CreateStrObject(std::string_view value = "");
		BoolObject *CreateBoolObject(bool value = false);
		NullObject *CreateNullObject();
		ArrayObject *CreateArrayObject(const std::vector<Object *> &elements = {});
		TableObject *CreateTableObject(const std::unordered_map<Object *, Object *> &elements = {});
		FieldObject *CreateFieldObject(std::string_view name, const std::unordered_map<std::string, Object *> &members, const std::vector<std::pair<std::string, FieldObject*>>& containedFields = {});
		FunctionObject *CreateFunctionObject(int64_t frameIdx);
		RefObject *CreateRefObject(std::string_view name,Object* index=nullptr);

		void Gc();

		std::function<Object *(std::vector<Object *>)> GetNativeFunction(std::string_view fnName);
		bool HasNativeFunction(std::string_view name);

		void PushObject(Object *object);
		Object *PopObject();

		void PushFrame(Frame *frame);
		Frame *PopFrame();
		bool IsFrameStackEmpty();

		uint64_t sp;
		std::array<Object *, STACK_MAX> m_ObjectStack;

		uint64_t fp;
		std::array<Frame *, STACK_MAX> m_FrameStack;

		Object *firstObject;
		int curObjCount;
		int maxObjCount;

		Context *m_Context;
	};
}