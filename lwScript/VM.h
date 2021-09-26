#pragma once
#include <array>
#include <cstdint>
#include <stack>
#include "Frame.h"
#include "Object.h"
#include "Utils.h"
#include "Environment.h"
namespace lws
{
#define STACK_MAX 256
#define INIT_OBJ_NUM_MAX 128

	class VM
	{
	public:
		VM();
		~VM();

		void ResetStatus();
		Object *Execute(const Frame &frame);

		void AddNativeFnObject(NativeFunctionObject *fn);
		Object *GetNativeFnObject(std::string_view fnName);

		NumObject *CreateNumObject(double value = 0.0);
		StrObject *CreateStrObject(std::string_view value = "");
		BoolObject *CreateBoolObject(bool value = false);
		NilObject *CreateNilObject();
		ArrayObject *CreateArrayObject(const std::vector<Object *> &elements = {});
		FunctionObject *CreateFunctionObject(int64_t frameIndex = 0);
		NativeFunctionObject *CreateNativeFunctionObject(std::string_view name = "", const std::function<Object *(std::vector<Object *>)> &function = nullptr);

		void Gc();
	private:

		void Push(Object *object);
		Object *Pop();


		uint8_t sp;
		std::array<Object *, STACK_MAX> m_Stack;

		Object *firstObject;
		int curObjCount;
		int maxObjCount;

		Environment *m_Environment;

		std::vector<NativeFunctionObject *> m_NativeFunctions;
	};
}