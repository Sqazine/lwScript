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
#include "Library.h"
namespace lws
{
#define STACK_MAX 2048
#define INIT_OBJ_NUM_MAX 2048

	class VM
	{
	public:
		VM();
		~VM();

		void ResetStatus();
		Object *Execute(Frame *frame);

		FloatingObject *CreateFloatingObject(double value = 0.0);
		IntegerObject *CreateIntegerObject(int64_t value = 0.0);
		StrObject *CreateStrObject(std::string_view value = "");
		BoolObject *CreateBoolObject(bool value = false);
		NilObject *CreateNilObject();
		ArrayObject *CreateArrayObject(const std::vector<Object *> &elements = {});
		TableObject *CreateTableObject(const std::unordered_map<Object *, Object *> &elements = {});
		ClassObject *CreateClassObject(Context *context);
		RefObject *CreateRefObject(std::string_view refName);

		void Gc();

	private:
		std::function<Object *(std::vector<Object *>)> GetNativeFunction(std::string_view fnName);
        bool HasNativeFunction(std::string_view name);

		void Push(Object *object);
		Object *Pop();

		uint8_t sp;
		std::array<Object *, STACK_MAX> m_Stack;

		Object *firstObject;
		int curObjCount;
		int maxObjCount;

		Context *m_Context;

		std::unordered_map<std::string,Library*> m_Libraries;
	};
}