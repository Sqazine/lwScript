#pragma once
#include <array>
#include <cstdint>
#include <stack>
#include "Frame.h"
#include "Object.h"
#include "Utils.h"
namespace lws
{
	class Environment
	{
	public:
		Environment() : m_UpEnvironment(nullptr) {}
		Environment(Environment *upEnvironment) : m_UpEnvironment(upEnvironment) {}
		~Environment() {}

		void DefineVariable(std::string_view name, Object *value)
		{
			auto iter = m_Values.find(name.data());
			if (iter != m_Values.end())
				Assert("Redefined variable:" + std::string(name) + " in current context.");
			else
				m_Values[name.data()] = value;
		}

		void AssignVariable(std::string_view name, Object *value)
		{
			auto iter = m_Values.find(name.data());
			if (iter != m_Values.end())
				m_Values[name.data()] = value;
			else if (m_UpEnvironment != nullptr)
				m_UpEnvironment->AssignVariable(name, value);
			else
				Assert("Undefine variable:" + std::string(name) + " in current context");
		}

		Object *GetVariable(std::string_view name)
		{
			auto iter = m_Values.find(name.data());

			if (iter != m_Values.end())
				return iter->second;

			if (m_UpEnvironment != nullptr)
				return m_UpEnvironment->GetVariable(name);

			return nilObject;
		}

		Environment *GetUpEnvironment() { return m_UpEnvironment; }

	private:
		std::unordered_map<std::string, Object *> m_Values;
		Environment *m_UpEnvironment;
	};

	class VM
	{
	public:
		VM();
		~VM();

		void ResetStatus();
		Object *Execute(const Frame &frame);

		void AddNativeFnObject(NativeFunctionObject *fn);
		Object *GetNativeFnObject(std::string_view fnName);

	private:
		void Push(Object *object);
		Object *Pop();

		uint8_t sp;
		std::array<Object *, 256> m_Stack;

		Environment *m_Environment;

		std::vector<NativeFunctionObject *> m_NativeFunctions;
	};

	VM::VM()
		: m_Environment(nullptr)
	{
		ResetStatus();

		m_NativeFunctions =
			{
				new NativeFunctionObject("print", [](std::vector<Object *> args) -> Object *
										 {
											 if (args.empty())
												 return nilObject;

											 if (args[0]->Type() != ObjectType::STR)
												 Assert("Invalid argument:The first argument of native print fn must be string type.");
											 if (args.size() == 1)
												 std::cout << args[0]->Stringify();
											 else //formatting output
											 {
												 std::string content = TO_STR_OBJ(args[0])->value;

												 int32_t pos = content.find("{}");
												 int32_t argpos = 1;
												 while (pos != std::string::npos)
												 {
													 if (argpos < args.size())
														 content.replace(pos, 2, args[argpos++]->Stringify());
													 else
														 content.replace(pos, 2, "nil");
													 pos = content.find("{}");
												 }

												 std::cout << content;
											 }
											 return nilObject;
										 }),
				new NativeFunctionObject("println", [](std::vector<Object *> args) -> Object *
										 {
											 if (args.empty())
												 return nilObject;

											 if (args[0]->Type() != ObjectType::STR)
												 Assert("Invalid argument:The first argument of native print fn must be string type.");
											 if (args.size() == 1)
												 std::cout << args[0]->Stringify() << std::endl;
											 else //formatting output
											 {
												 std::string content = TO_STR_OBJ(args[0])->value;

												 int32_t pos = content.find("{}");
												 int32_t argpos = 1;
												 while (pos != std::string::npos)
												 {
													 if (argpos < args.size())
														 content.replace(pos, 2, args[argpos++]->Stringify());
													 else
														 content.replace(pos, 2, "nil");
													 pos = content.find("{}");
												 }

												 std::cout << content << std::endl;
											 }
											 return nilObject;
										 }),
			};
	}
	VM::~VM()
	{
		if (m_Environment)
		{
			delete m_Environment;
			m_Environment = nullptr;
		}
	}

	void VM::AddNativeFnObject(NativeFunctionObject *fn)
	{
		for (const auto &f : m_NativeFunctions)
			if (f->name == fn->name)
				std::cout << "Redefinite native function:" << fn->name;
		m_NativeFunctions.emplace_back(fn);
	}

	Object *VM::GetNativeFnObject(std::string_view fnName)
	{
		for (const auto &fnObj : m_NativeFunctions)
			if (fnObj->name == fnName)
				return fnObj;

		return nilObject;
	}

	Object *VM::Execute(const Frame &frame)
	{
// + - * /
#define COMMON_BINARY(op)                                                                  \
	do                                                                                     \
	{                                                                                      \
		Object *left = Pop();                                                              \
		Object *right = Pop();                                                             \
		if (IS_NUM_OBJ(right) && IS_NUM_OBJ(left))                                         \
		{                                                                                  \
			auto t = new NumObject();                                                      \
			t->value = TO_NUM_OBJ(left)->value op TO_NUM_OBJ(right)->value;                \
			Push(t);                                                                       \
		}                                                                                  \
		else                                                                               \
			Assert("Invalid binary op:" + left->Stringify() + (#op) + right->Stringify()); \
	} while (0);

// > >= < <= == !=
#define COMPARE_BINARY(op)                                                                          \
	do                                                                                              \
	{                                                                                               \
		Object *left = Pop();                                                                       \
		Object *right = Pop();                                                                      \
		if (IS_NUM_OBJ(right) && IS_NUM_OBJ(left))                                                  \
			Push(TO_NUM_OBJ(left)->value op TO_NUM_OBJ(right)->value ? trueObject : falseObject);   \
		else if (IS_BOOL_OBJ(right) && IS_BOOL_OBJ(left))                                           \
			Push(TO_BOOL_OBJ(left)->value op TO_BOOL_OBJ(right)->value ? trueObject : falseObject); \
		else if (IS_NIL_OBJ(right) && IS_NIL_OBJ(left))                                             \
			Push(TO_NIL_OBJ(left) op TO_NIL_OBJ(right) ? trueObject : falseObject);                 \
		else                                                                                        \
			Push(falseObject);                                                                      \
	} while (0);

#define LOGIC_BINARY(op)                                                                                 \
	do                                                                                                   \
	{                                                                                                    \
		Object *left = Pop();                                                                            \
		Object *right = Pop();                                                                           \
		if (IS_BOOL_OBJ(right) && IS_BOOL_OBJ(left))                                                     \
			Push(((BoolObject *)left)->value op((BoolObject *)right)->value ? trueObject : falseObject); \
		else                                                                                             \
			Assert("Invalid op:" + left->Stringify() + (#op) + right->Stringify());                      \
	} while (0);

		for (size_t ip = 0; ip < frame.m_Codes.size(); ++ip)
		{
			uint8_t instruction = frame.m_Codes[ip];
			switch (instruction)
			{
			case OP_RETURN:
				return Pop();
				break;
			case OP_PUSH:
				Push(frame.m_Objects[frame.m_Codes[++ip]]);
				break;
			case OP_NEG:
			{
				Object *object = Pop();
				if (IS_NUM_OBJ(object))
				{
					auto t = TO_NUM_OBJ(object);
					t->value *= -1.0;
					Push(t);
				}
				else
					Assert("Invalid op:'-'" + object->Stringify());
				break;
			}
			case OP_ADD:
				COMMON_BINARY(+);
				break;
			case OP_SUB:
				COMMON_BINARY(-);
				break;
			case OP_MUL:
				COMMON_BINARY(*);
				break;
			case OP_DIV:
				COMMON_BINARY(*);
				break;
			case OP_GT:
				COMPARE_BINARY(>);
				break;
			case OP_LE:
				COMPARE_BINARY(<);
				break;
			case OP_GTEQ:
				COMPARE_BINARY(>=);
				break;
			case OP_LEEQ:
				COMPARE_BINARY(<=);
				break;
			case OP_EQ:
				COMPARE_BINARY(==);
				break;
			case OP_AND:
				LOGIC_BINARY(&&);
				break;
			case OP_OR:
				LOGIC_BINARY(||);
				break;
			case OP_DEFINE_VAR:
			{
				Object *variableName = Pop();
				Object *value = Pop();
				m_Environment->DefineVariable(((StrObject *)variableName)->value, value);
				break;
			}
			case OP_SET_VAR:
			{
				std::string name = TO_STR_OBJ(Pop())->value;
				Object *value = Pop();
				Object *variableObject = m_Environment->GetVariable(name);
				m_Environment->AssignVariable(name, value);
				break;
			}
			case OP_GET_VAR:
			{
				std::string name = TO_STR_OBJ(Pop())->value;
				Object *variableObject = m_Environment->GetVariable(name);
				//not variable
				if (IS_NIL_OBJ(variableObject))
				{
					//search native function
					variableObject = GetNativeFnObject(name);
					if (IS_NIL_OBJ(variableObject))
						Assert("No variable or function:" + std::string(name));
				}
				Push(variableObject);
				break;
			}
			case OP_DEFINE_ARRAY:
			{
				ArrayObject *arrayObject = new ArrayObject();
				NumObject *arraySize = TO_NUM_OBJ(Pop());
				for (int64_t i = 0; i < (int64_t)arraySize->value; ++i)
					arrayObject->elements.insert(arrayObject->elements.begin(), Pop());
				Push(arrayObject);
				break;
			}
			case OP_GET_INDEX_VAR:
			{
				Object *object = Pop();
				Object *index = Pop();
				if (IS_ARRAY_OBJ(object))
				{
					ArrayObject *arrayObject = TO_ARRAY_OBJ(object);
					if (index->Type() != ObjectType::NUM)
						Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index->Stringify());

					int64_t iIndex = (int64_t)TO_NUM_OBJ(index)->value;

					if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
						Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

					Push(arrayObject->elements[iIndex]);
				}
				else
					Assert("Invalid index op.The indexed object isn't a array object or a table object:" + object->Stringify());
				break;
			}
			case OP_SET_INDEX_VAR:
			{
				Object *object = Pop();
				Object *index = Pop();
				Object *assigner = Pop();

				if (IS_ARRAY_OBJ(object))
				{
					ArrayObject *arrayObject = TO_ARRAY_OBJ(object);
					if (IS_NUM_OBJ(index))
						Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index->Stringify());

					int64_t iIndex = (int64_t)TO_NUM_OBJ(index)->value;

					if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
						Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

					arrayObject->elements[iIndex] = assigner;
				}
				else
					Assert("Invalid index op.The indexed object isn't a array object or a table object:" + object->Stringify());
				break;
			}
			case OP_ENTER_SCOPE:
			{
				m_Environment = new Environment(m_Environment);
				break;
			}
			case OP_EXIT_SCOPE:
			{
				Environment *tmp = m_Environment->GetUpEnvironment();
				delete m_Environment;
				m_Environment = tmp;
				break;
			}
			case OP_JUMP_IF_FALSE:
			{
				uint64_t address = (uint64_t)(TO_NUM_OBJ(Pop())->value);
				bool isJump = !TO_BOOL_OBJ(Pop())->value;

				if (isJump)
					ip = address;
				break;
			}
			case OP_JUMP:
			{
				uint64_t address = (uint64_t)(TO_NUM_OBJ(Pop())->value);
				ip = address;
				break;
			}
			case OP_FUNCTION_CALL:
			{
				NumObject *argCount = TO_NUM_OBJ(Pop());
				Object *fn = Pop();
				if (IS_STR_OBJ(fn))
					fn = m_Environment->GetVariable(TO_STR_OBJ(fn)->value);

				if (IS_FUNCTION_OBJ(fn))
					Push(Execute(frame.m_FunctionFrames[TO_FUNCTION_OBJ(fn)->frameIndex]));
				else if (IS_NATIVE_FUNCTION_OBJ(fn))
				{
					std::vector<Object *> args;
					for (size_t i = 0; i < argCount->value; ++i)
						args.insert(args.begin(), Pop());

					Object *natFnObj = GetNativeFnObject(TO_STR_OBJ(fn)->value);

					if (IS_NIL_OBJ(natFnObj))
						Assert("No native function:" + TO_STR_OBJ(fn)->value);

					Push(TO_NATIVE_FUNCTION_OBJ(natFnObj)->function(args));
				}
				else
					Assert("No function:" + fn->Stringify());
				break;
			}
			default:
				break;
			}
		}

		return nilObject;
	}
	void VM::ResetStatus()
	{
		sp = 0;
		std::array<Object *, 256>().swap(m_Stack);

		if (m_Environment != nullptr)
		{
			delete m_Environment;
			m_Environment = nullptr;
		}
		m_Environment = new Environment();
	}
	void VM::Push(Object *object)
	{
		m_Stack[sp++] = object;
	}
	Object *VM::Pop()
	{
		return m_Stack[--sp];
	}
}