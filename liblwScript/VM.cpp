#include "VM.h"
#include <iostream>
#include "Utils.h"
#include "Object.h"
namespace lws
{
	Value VM::sNullValue = Value();

	VM::VM()
		: mObjectChain(nullptr), mOpenUpValues(nullptr), mStackTop(nullptr)
	{
		ResetStatus();
	}
	VM::~VM()
	{
		FreeObjects();
	}

	void VM::ResetStatus()
	{
		if (!mObjectChain)
			FreeObjects();

		mFrameCount = 0;
		mBytesAllocated = 0;
		mNextGCByteSize = 256;
		mStackTop = mValueStack;
		mObjectChain = nullptr;
		mOpenUpValues = nullptr;

		for (int32_t i = 0; i < LibraryManager::Instance().mStdLibraries.size(); ++i)
			mGlobalVariables[i] = LibraryManager::Instance().mStdLibraries[i]->Clone();
	}

	std::vector<Value> VM::Run(FunctionObject *mainFunc)
	{
		ResetStatus();

		RegisterToGCRecordChain(mainFunc);

		auto closure = CreateObject<ClosureObject>(mainFunc);

		Push(closure);

		CallFrame *mainCallFrame = &mFrames[mFrameCount++];
		mainCallFrame->closure = closure;
		mainCallFrame->ip = closure->function->chunk.opCodes.data();
		mainCallFrame->slots = mStackTop - 1;

		Execute();

		std::vector<Value> returnValues;

		while (mStackTop != mValueStack + 1)
			returnValues.emplace_back(Pop());

		Pop();

		return returnValues;
	}

	void VM::Execute()
	{
		//  - * /
#define COMMON_BINARY(op)                                                                 \
	do                                                                                    \
	{                                                                                     \
		Value right = Pop();                                                              \
		Value left = Pop();                                                               \
		if (IS_REF_VALUE(left))                                                           \
			left = *TO_REF_VALUE(left)->pointer;                                          \
		if (IS_REF_VALUE(right))                                                          \
			right = *TO_REF_VALUE(right)->pointer;                                        \
		if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                                    \
			Push(TO_INT_VALUE(left) op TO_INT_VALUE(right));                              \
		else if (IS_REAL_VALUE(left) && IS_REAL_VALUE(right))                             \
			Push(TO_REAL_VALUE(left) op TO_REAL_VALUE(right));                            \
		else if (IS_INT_VALUE(left) && IS_REAL_VALUE(right))                              \
			Push(TO_INT_VALUE(left) op TO_REAL_VALUE(right));                             \
		else if (IS_REAL_VALUE(left) && IS_INT_VALUE(right))                              \
			Push(TO_REAL_VALUE(left) op TO_INT_VALUE(right));                             \
		else                                                                              \
			ASSERT(L"Invalid binary op:" + left.Stringify() + (L#op) + right.Stringify()) \
	} while (0);

// & | << >>
#define INTEGER_BINARY(op)                                                                \
	do                                                                                    \
	{                                                                                     \
		Value right = Pop();                                                              \
		Value left = Pop();                                                               \
		if (IS_REF_VALUE(left))                                                           \
			left = *TO_REF_VALUE(left)->pointer;                                          \
		if (IS_REF_VALUE(right))                                                          \
			right = *TO_REF_VALUE(right)->pointer;                                        \
		if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                                    \
			Push(TO_INT_VALUE(left) op TO_INT_VALUE(right));                              \
		else                                                                              \
			ASSERT(L"Invalid binary op:" + left.Stringify() + (L#op) + right.Stringify()) \
	} while (0);

// > <
#define COMPARE_BINARY(op)                                                    \
	do                                                                        \
	{                                                                         \
		Value right = Pop();                                                  \
		Value left = Pop();                                                   \
		if (IS_REF_VALUE(left))                                               \
			left = *TO_REF_VALUE(left)->pointer;                              \
		if (IS_REF_VALUE(right))                                              \
			right = *TO_REF_VALUE(right)->pointer;                            \
		if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                        \
			Push(TO_INT_VALUE(left) op TO_INT_VALUE(right) ? true : false);   \
		else if (IS_REAL_VALUE(left) && IS_REAL_VALUE(right))                 \
			Push(TO_REAL_VALUE(left) op TO_REAL_VALUE(right) ? true : false); \
		else if (IS_INT_VALUE(left) && IS_REAL_VALUE(right))                  \
			Push(TO_INT_VALUE(left) op TO_REAL_VALUE(right) ? true : false);  \
		else if (IS_REAL_VALUE(left) && IS_INT_VALUE(right))                  \
			Push(TO_REAL_VALUE(left) op TO_INT_VALUE(right) ? true : false);  \
		else                                                                  \
			Push(false);                                                      \
	} while (0);

// && ||
#define LOGIC_BINARY(op)                                                                    \
	do                                                                                      \
	{                                                                                       \
		Value right = Pop();                                                                \
		Value left = Pop();                                                                 \
		if (IS_REF_VALUE(left))                                                             \
			left = *TO_REF_VALUE(left)->pointer;                                            \
		if (IS_REF_VALUE(right))                                                            \
			right = *TO_REF_VALUE(right)->pointer;                                          \
		if (IS_BOOL_VALUE(left) && IS_BOOL_VALUE(right))                                    \
			Push(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false)); \
		else                                                                                \
			ASSERT("Invalid op:" + left.Stringify() + (L#op) + right.Stringify())           \
	} while (0);

#define READ_INS() (*frame->ip++)

		CallFrame *frame = &mFrames[mFrameCount - 1];

		while (1)
		{
			auto instruction = READ_INS();
			switch (instruction)
			{
			case OP_RETURN:
			{
				auto retCount = READ_INS();
				Value *retValues;
				retValues = mStackTop - retCount;

				ClosedUpValues(frame->slots);

				mFrameCount--;
				if (mFrameCount == 0)
					return;

				mStackTop = frame->slots;

				if (retCount == 0)
					Push(sNullValue);
				else
				{
					uint8_t i = 0;
					while (i < retCount)
					{
						auto value = *(retValues + i);
						Push(value);
						i++;
					}
				}

				frame = &mFrames[mFrameCount - 1];
				break;
			}
			case OP_CONSTANT:
			{
				auto pos = READ_INS();
				auto v = frame->closure->function->chunk.constants[pos];
				auto vClone = v.Clone();
				RegisterToGCRecordChain(vClone);
				Push(vClone);
				break;
			}
			case OP_NULL:
			{
				Push(sNullValue);
				break;
			}
			case OP_SET_GLOBAL:
			{
				auto pos = READ_INS();
				auto v = Peek(0);
				if (IS_REF_VALUE(mGlobalVariables[pos]))
					*TO_REF_VALUE(mGlobalVariables[pos])->pointer = v;
				else
					mGlobalVariables[pos] = v;
				break;
			}
			case OP_GET_GLOBAL:
			{
				auto pos = READ_INS();
				Push(mGlobalVariables[pos]);
				break;
			}
			case OP_SET_LOCAL:
			{
				auto pos = READ_INS();
				auto value = Peek(0);

				auto slot = frame->slots + pos;

				if (IS_REF_VALUE((*slot)))
					*TO_REF_VALUE((*slot))->pointer = value;
				else
					*slot = value; // now assume base ptr on the stack bottom
				break;
			}
			case OP_GET_LOCAL:
			{
				auto pos = READ_INS();
				Push(frame->slots[pos]); // now assume base ptr on the stack bottom
				break;
			}
			case OP_SET_UPVALUE:
			{
				auto pos = READ_INS();
				auto v = Peek(0);
				*frame->closure->upvalues[pos]->location = Peek(0);
				break;
			}
			case OP_GET_UPVALUE:
			{
				auto pos = READ_INS();
				Push(*frame->closure->upvalues[pos]->location);
				break;
			}
			case OP_CLOSE_UPVALUE:
			{
				ClosedUpValues(mStackTop - 1);
				Pop();
				break;
			}
			case OP_ADD:
			{
				Value left = Pop();
				Value right = Pop();
				if (IS_REF_VALUE(left))
					left = *TO_REF_VALUE(left)->pointer;
				if (IS_REF_VALUE(right))
					right = *TO_REF_VALUE(right)->pointer;
				if (IS_INT_VALUE(left) && IS_INT_VALUE(right))
					Push(TO_INT_VALUE(left) + TO_INT_VALUE(right));
				else if (IS_REAL_VALUE(left) && IS_REAL_VALUE(right))
					Push(TO_REAL_VALUE(left) + TO_REAL_VALUE(right));
				else if (IS_INT_VALUE(left) && IS_REAL_VALUE(right))
					Push(TO_INT_VALUE(left) + TO_REAL_VALUE(right));
				else if (IS_REAL_VALUE(left) && IS_INT_VALUE(right))
					Push(TO_REAL_VALUE(left) + TO_INT_VALUE(right));
				else if (IS_STR_VALUE(left) && IS_STR_VALUE(right))
					Push(CreateObject<StrObject>(TO_STR_VALUE(left) + TO_STR_VALUE(right)));
				else
					ASSERT(L"Invalid binary op:" + left.Stringify() + L" + " + right.Stringify())
				break;
			}
			case OP_SUB:
			{
				COMMON_BINARY(-);
				break;
			}
			case OP_MUL:
			{
				COMMON_BINARY(*);
				break;
			}
			case OP_DIV:
			{
				COMMON_BINARY(/);
				break;
			}
			case OP_MOD:
			{
				INTEGER_BINARY(%);
				break;
			}
			case OP_BIT_AND:
			{
				INTEGER_BINARY(&);
				break;
			}
			case OP_BIT_OR:
			{
				INTEGER_BINARY(|);
				break;
			}
			case OP_BIT_LEFT_SHIFT:
			{
				INTEGER_BINARY(<<);
				break;
			}
			case OP_BIT_RIGHT_SHIFT:
			{
				INTEGER_BINARY(>>);
				break;
			}
			case OP_LESS:
			{
				COMPARE_BINARY(<);
				break;
			}
			case OP_GREATER:
			{
				COMPARE_BINARY(>);
				break;
			}
			case OP_NOT:
			{
				auto value = Pop();
				if (IS_REF_VALUE(value))
					value = *TO_REF_VALUE(value)->pointer;
				if (!IS_BOOL_VALUE(value))
					ASSERT(L"Invalid op:! " + value.Stringify() + L", only !(bool expr) is available.")
				Push(!TO_BOOL_VALUE(value));
				break;
			}
			case OP_EQUAL:
			{
				Value left = Pop();
				Value right = Pop();
				if (IS_REF_VALUE(left))
					left = *TO_REF_VALUE(left)->pointer;
				if (IS_REF_VALUE(right))
					right = *TO_REF_VALUE(right)->pointer;
				Push(left == right);
				break;
			}
			case OP_MINUS:
			{
				auto value = Pop();
				if (IS_REF_VALUE(value))
					value = *TO_REF_VALUE(value)->pointer;
				if (IS_INT_VALUE(value))
					Push(-TO_INT_VALUE(value));
				else if (IS_REAL_VALUE(value))
					Push(-TO_REAL_VALUE(value));
				else
					ASSERT(L"Invalid op:-" + value.Stringify() + L", only -(int||real expr) is available.")
				break;
			}
			case OP_FACTORIAL:
			{
				auto value = Pop();
				if (IS_REF_VALUE(value))
					value = *TO_REF_VALUE(value)->pointer;
				if (IS_INT_VALUE(value))
					Push(Factorial(TO_INT_VALUE(value)));
				else
					ASSERT(L"Invalid op:" + value.Stringify() + L"!, only (int expr)! is available.")
				break;
			}
			case OP_ARRAY:
			{
				auto count = READ_INS();

				std::vector<Value> elements;
				auto prePtr = mStackTop - count;
				for (auto i = 0; i < count; ++i)
					elements.emplace_back(*prePtr++);
				mStackTop -= count;
				auto arrayObject = CreateObject<ArrayObject>(elements);
				Push(arrayObject);
				break;
			}
			case OP_DICT:
			{
				auto eCount = READ_INS();
				DictObject::ValueUnorderedMap elements;
				for (int64_t i = 0; i < (int64_t)eCount; ++i)
				{
					auto key = Pop();
					auto value = Pop();
					elements[key] = value;
				}
				Push(CreateObject<DictObject>(elements));
				break;
			}
			case OP_GET_INDEX:
			{
				auto idxValue = Pop();
				auto dsValue = Pop();
				if (IS_ARRAY_VALUE(dsValue))
				{
					auto array = TO_ARRAY_VALUE(dsValue);
					if (!IS_INT_VALUE(idxValue))
						ASSERT("Invalid idx for array,only integer is available.")
					auto intIdx = TO_INT_VALUE(idxValue);

					if (intIdx < 0)
						intIdx = (int64_t)array->elements.size() + intIdx;

					if (intIdx < 0 || intIdx >= (int64_t)array->elements.size())
						ASSERT("Idx out of range.")
					Push(array->elements[intIdx]);
				}
				else if (IS_STR_VALUE(dsValue))
				{
					auto str = TO_STR_VALUE(dsValue);
					if (!IS_INT_VALUE(idxValue))
						ASSERT("Invalid idx for array,only integer is available.")

					auto intIdx = TO_INT_VALUE(idxValue);

					if (intIdx < 0)
						intIdx = (int64_t)str.size() + intIdx;

					if (intIdx < 0 || intIdx >= (int64_t)str.size())
						ASSERT("Idx out of range.")
					Push(CreateObject<StrObject>(str.substr(intIdx, 1)));
				}
				else if (IS_DICT_VALUE(dsValue))
				{
					auto dict = TO_DICT_VALUE(dsValue);

					auto iter = dict->elements.find(idxValue);

					if (iter != dict->elements.end())
						Push(iter->second);
					else
						ASSERT("No key in dict.")
				}
				break;
			}
			case OP_SET_INDEX:
			{
				auto idxValue = Pop();
				auto dsValue = Pop();
				auto newValue = Peek(0);
				if (IS_ARRAY_VALUE(dsValue))
				{
					auto array = TO_ARRAY_VALUE(dsValue);
					if (!IS_INT_VALUE(idxValue))
						ASSERT(L"Invalid idx for array,only integer is available.")
					auto intIdx = TO_INT_VALUE(idxValue);

					if (intIdx < 0 || intIdx >= (int64_t)array->elements.size())
						ASSERT("Idx out of range.")

					if (intIdx < 0 || intIdx >= (int64_t)array->elements.size())
						ASSERT(L"Idx out of range.")
					array->elements[intIdx] = newValue;
				}
				else if (IS_STR_VALUE(dsValue))
				{
					auto str = TO_STR_VALUE(dsValue);
					if (!IS_INT_VALUE(idxValue))
						ASSERT(L"Invalid idx for array,only integer is available.")

					auto intIdx = TO_INT_VALUE(idxValue);
					if (intIdx < 0 || intIdx >= (int64_t)str.size())
						ASSERT("Idx out of range.")

					if (!IS_STR_VALUE(newValue))
						ASSERT(L"Cannot insert a non string clip:" + newValue.Stringify() + L" to string:" + str)

					str.append(TO_STR_VALUE(newValue), intIdx, TO_STR_VALUE(newValue).size());
				}
				else if (IS_DICT_VALUE(dsValue))
				{
					auto dict = TO_DICT_VALUE(dsValue);
					dict->elements[idxValue] = newValue;
				}
				break;
			}
			case OP_POP:
			{
				Pop();
				break;
			}
			case OP_JUMP_IF_FALSE:
			{
				uint16_t address = (*(frame->ip++) << 8) | (*(frame->ip++));
				if (IsFalsey(Peek(0)))
					frame->ip += address;
				break;
			}
			case OP_JUMP:
			{
				uint16_t address = (*(frame->ip++) << 8) | (*(frame->ip++));
				frame->ip += address;
				break;
			}
			case OP_LOOP:
			{
				uint16_t address = (*(frame->ip++) << 8) | (*(frame->ip++));
				frame->ip -= address;
				break;
			}
			case OP_REF_GLOBAL:
			{
				auto index = READ_INS();
				Push(CreateObject<RefObject>(&mGlobalVariables[index]));
				break;
			}
			case OP_REF_LOCAL:
			{
				auto index = READ_INS();
				Push(CreateObject<RefObject>(frame->slots + index));
				break;
			}
			case OP_REF_UPVALUE:
			{
				auto index = READ_INS();
				Push(CreateObject<RefObject>(frame->closure->upvalues[index]->location));
				break;
			}
			case OP_REF_INDEX_GLOBAL:
			{
				auto index = READ_INS();
				auto idxValue = Pop();
				if (IS_DICT_VALUE(mGlobalVariables[index]))
				{
					Push(CreateObject<RefObject>(&TO_DICT_VALUE(mGlobalVariables[index])->elements[idxValue]));
				}
				else if (IS_ARRAY_VALUE(mGlobalVariables[index]))
				{
					auto array = TO_ARRAY_VALUE(mGlobalVariables[index]);

					if (!IS_INT_VALUE(idxValue))
						ASSERT(L"Invalid idx for array,only integer is available.")
					auto intIdx = TO_INT_VALUE(idxValue);

					if (intIdx < 0)
						intIdx = (int64_t)array->elements.size() + intIdx;

					if (intIdx < 0 || intIdx >= array->elements.size())
						ASSERT(L"Idx out of range.")
					Push(CreateObject<RefObject>(&(array->elements[intIdx])));
				}
				else
					ASSERT(L"Invalid indexed reference type:" + mGlobalVariables[index].Stringify() + L" not a dict or array value.")
				break;
			}
			case OP_REF_INDEX_LOCAL:
			{
				auto index = READ_INS();
				auto idxValue = Pop();
				Value *v = frame->slots + index;
				if (IS_DICT_VALUE((*v)))
				{
					Push(CreateObject<RefObject>(&TO_DICT_VALUE((*v))->elements[idxValue]));
				}
				else if (IS_ARRAY_VALUE((*v)))
				{
					auto array = TO_ARRAY_VALUE((*v));

					if (!IS_INT_VALUE(idxValue))
						ASSERT(L"Invalid idx for array,only integer is available.")
					auto intIdx = TO_INT_VALUE(idxValue);

					if (intIdx < 0)
						intIdx = (int64_t)array->elements.size() + intIdx;

					if (intIdx < 0 || intIdx >= array->elements.size())
						ASSERT(L"Idx out of range.")
					Push(CreateObject<RefObject>(&array->elements[intIdx]));
				}
				else
					ASSERT(L"Invalid indexed reference type:" + v->Stringify() + L" not a dict or array value.")
				break;
			}
			case OP_REF_INDEX_UPVALUE:
			{
				auto index = READ_INS();
				auto idxValue = Pop();
				Value *v = frame->closure->upvalues[index]->location;
				if (IS_DICT_VALUE((*v)))
				{
					Push(CreateObject<RefObject>(&TO_DICT_VALUE((*v))->elements[idxValue]));
				}
				else if (IS_ARRAY_VALUE((*v)))
				{
					auto array = TO_ARRAY_VALUE((*v));
					if (!IS_INT_VALUE(idxValue))
						ASSERT(L"Invalid idx for array,only integer is available.")
					auto intIdx = TO_INT_VALUE(idxValue);

					if (intIdx < 0)
						intIdx = (int64_t)array->elements.size() + intIdx;

					if (intIdx < 0 || intIdx >= array->elements.size())
						ASSERT(L"Idx out of range.")
					Push(CreateObject<RefObject>(&array->elements[intIdx]));
				}
				else
					ASSERT(L"Invalid indexed reference type:" + v->Stringify() + L" not a dict or array value.")
				break;
			}
			case OP_CALL:
			{
				auto argCount = READ_INS();
				auto callee = Peek(argCount);
				if (IS_CLOSURE_VALUE(callee) || IS_CLASS_CLOSURE_BIND_VALUE(callee)) // normal function or class member function
				{
					if (IS_CLASS_CLOSURE_BIND_VALUE(callee))
					{
						auto binding = TO_CLASS_CLOSURE_BIND_VALUE(callee);
						mStackTop[-argCount - 1] = binding->receiver;
						callee = binding->closure;
					}

					if (argCount != TO_CLOSURE_VALUE(callee)->function->arity)
						ASSERT(L"No matching argument count.")
					// init a new frame
					CallFrame *newframe = &mFrames[mFrameCount++];
					newframe->closure = TO_CLOSURE_VALUE(callee);
					newframe->ip = newframe->closure->function->chunk.opCodes.data();
					newframe->slots = mStackTop - argCount - 1;

					frame = &mFrames[mFrameCount - 1];
				}
				else if (IS_CLASS_VALUE(callee)) // class constructor
				{
					auto klass = TO_CLASS_VALUE(callee);
					// no user-defined constructor and calling none argument construction
					// like: class A{} let a=new A();
					// skip calling constructor(because class object has been instantiated)
					if (argCount == 0 && klass->constructors.size() == 0)
						break;
					else
					{
						auto iter = klass->constructors.find(argCount);
						if (iter == klass->constructors.end())
							ASSERT(L"Not matching argument count of class:" + klass->name + L"'s constructors.");

						auto ctor = iter->second;
						// init a new frame
						CallFrame *newframe = &mFrames[mFrameCount++];
						newframe->closure = ctor;
						newframe->ip = newframe->closure->function->chunk.opCodes.data();
						newframe->slots = mStackTop - argCount - 1;

						frame = &mFrames[mFrameCount - 1];
					}
				}
				else if (IS_NATIVE_FUNCTION_VALUE(callee)) // native function
				{
					std::vector<Value> args(argCount);

					int32_t j = 0;
					for (Value *slot = mStackTop - argCount; slot < mStackTop && j < argCount; ++slot, ++j)
						args[j] = *slot;

					mStackTop -= argCount + 1;

					auto retV = TO_NATIVE_FUNCTION_VALUE(callee)->fn(args);
					Push(retV);
				}
				else
					ASSERT(L"Invalid callee,Only function is available:" + callee.Stringify())
				break;
			}
			case OP_CLASS:
			{
				auto name = Peek(0);
				auto ctorCount = READ_INS();
				auto varCount = READ_INS();
				auto constCount = READ_INS();
				auto parentClassCount = READ_INS();

				auto classObj = CreateObject<ClassObject>();
				classObj->name = TO_STR_VALUE(name);
				Pop(); // pop name strobject

				for (int32_t i = 0; i < ctorCount; ++i)
				{
					auto v = TO_CLOSURE_VALUE(Pop());
					classObj->constructors[v->function->arity] = v;
				}

				for (int32_t i = 0; i < parentClassCount; ++i)
				{
					name = Pop();
					auto parentClass = Pop();
					classObj->parents[TO_STR_VALUE(name)] = TO_CLASS_VALUE(parentClass);
				}

				for (int32_t i = 0; i < constCount; ++i)
				{
					name = Pop();
					auto v = Pop();
					v.desc = DESC_CONSTANT;
					classObj->members[TO_STR_VALUE(name)] = v;
				}

				for (int32_t i = 0; i < varCount; ++i)
				{
					name = Pop();
					auto v = Pop();
					v.desc = DESC_VARIABLE;
					classObj->members[TO_STR_VALUE(name)] = v;
				}

				Push(classObj);
				break;
			}
			case OP_ANONYMOUS_OBJ:
			{
				auto eCount = READ_INS();
				std::unordered_map<std::wstring, Value> elements;
				for (int64_t i = 0; i < (int64_t)eCount; ++i)
				{
					auto key = TO_STR_VALUE(Pop());
					auto value = Pop();
					elements[key] = value;
				}
				Push(CreateObject<AnonymousObject>(elements));
				break;
			}
			case OP_GET_PROPERTY:
			{
				auto peekValue = Peek(1);

				if (IS_REF_VALUE(peekValue))
					peekValue = *(TO_REF_VALUE(peekValue)->pointer);

				auto propName = TO_STR_VALUE(Pop());
				if (IS_CLASS_VALUE(peekValue))
				{
					ClassObject *klass = TO_CLASS_VALUE(peekValue);

					Value member;
					if (klass->GetMember(propName, member))
					{
						Pop(); // pop class object
						if (IS_CLOSURE_VALUE(member))
						{
							ClassClosureBindObject *binding = CreateObject<ClassClosureBindObject>(klass, TO_CLOSURE_VALUE(member));
							member = Value(binding);
						}
						Push(member);
						break;
					}
					else
						ASSERT(L"No member:" + propName + L" in class object" + klass->name)
				}
				else if (IS_ENUM_VALUE(peekValue))
				{
					EnumObject *enumObj = TO_ENUM_VALUE(peekValue);

					Value member;
					if (enumObj->GetMember(propName, member))
					{
						Pop(); // pop enum object
						Push(member);
						break;
					}
					else
						ASSERT(L"No member:" + propName + L" in enum object " + enumObj->name)
				}
				else if (IS_ANONYMOUS_VALUE(peekValue))
				{
					auto anonymousObj = TO_ANONYMOUS_VALUE(peekValue);
					auto iter = anonymousObj->elements.find(propName);
					if (iter == anonymousObj->elements.end())
						ASSERT(L"No property:" + propName + L" in anonymous object.");
					Pop(); //pop anonymouse object
					Push(iter->second);
					break;
				}
				else
					ASSERT(L"Invalid call:not a valid class,enum or anonymous object instance:" + peekValue.Stringify())

				break;
			}
			case OP_SET_PROPERTY:
			{
				auto peekValue = Peek(1);

				if (IS_REF_VALUE(peekValue))
					peekValue = *(TO_REF_VALUE(peekValue)->pointer);

				auto propName = TO_STR_VALUE(Pop());
				if (IS_CLASS_VALUE(peekValue))
				{
					auto klass = TO_CLASS_VALUE(peekValue);
					Pop(); // pop class value

					Value member;
					if (klass->GetMember(propName, member))
					{
						if (member.desc == DESC_CONSTANT)
							ASSERT(L"Constant cannot be assigned twice:" + klass->name + L"'s member:" + propName + L" is a constant value")
						else
							klass->members[propName] = Peek(0);
					}
					else
						ASSERT(L"No member named:" + propName + L"in class:" + klass->name)
				}
				else if (IS_ANONYMOUS_VALUE(peekValue))
				{
					auto anonymousObj = TO_ANONYMOUS_VALUE(peekValue);
					auto iter = anonymousObj->elements.find(propName);
					if (iter == anonymousObj->elements.end())
						ASSERT(L"No property:" + propName + L"in anonymous object.");
					Pop(); //pop anonymouse object
					anonymousObj->elements[iter->first] = Peek(0);
					break;
				}
				else if (IS_ENUM_VALUE(peekValue))
					ASSERT(L"Invalid call:cannot assign value to a enum object member.")
				else
					ASSERT(L"Invalid call:not a valid class or anonymous object instance.")
				break;
			}
			case OP_GET_BASE:
			{
				if (!IS_CLASS_VALUE(Peek(1)))
					ASSERT(L"Invalid class call:not a valid class instance.")
				auto propName = TO_STR_VALUE(Pop());
				auto klass = TO_CLASS_VALUE(Pop());
				Value member;
				bool hasValue = klass->GetParentMember(propName, member);
				if (!hasValue)
					ASSERT(L"No member:" + propName + L"in class:" + klass->name + L" 's parent class(es).")
				Push(member);
				break;
			}
			case OP_CLOSURE:
			{
				auto pos = READ_INS();
				auto func = TO_FUNCTION_VALUE(frame->closure->function->chunk.constants[pos]);
				auto closure = CreateObject<ClosureObject>(func);

				for (int32_t i = 0; i < closure->upvalues.size(); ++i)
				{
					auto index = READ_INS();
					auto depth = READ_INS();
					if (depth == mFrameCount - 1)
					{
						auto captured = CaptureUpValue(frame->slots + index);
						closure->upvalues[i] = captured;
					}
					else
						closure->upvalues[i] = frame->closure->upvalues[index];
				}

				Push(closure);
				break;
			}
			default:
				break;
			}
		}
	}

	bool VM::IsFalsey(const Value &v)
	{
		return IS_NULL_VALUE(v) || (IS_BOOL_VALUE(v) && !TO_BOOL_VALUE(v));
	}

	void VM::Push(const Value &value)
	{
		*(mStackTop++) = value;
	}
	Value VM::Pop()
	{
		return *(--mStackTop);
	}

	Value VM::Peek(int32_t distance)
	{
		return *(mStackTop - distance - 1);
	}

	UpValueObject *VM::CaptureUpValue(Value *location)
	{
		UpValueObject *prevUpValue = nullptr;
		UpValueObject *upValue = mOpenUpValues;

		while (upValue != nullptr && upValue->location > location)
		{
			prevUpValue = upValue;
			upValue = upValue->nextUpValue;
		}

		if (upValue != nullptr && upValue->location == location)
			return upValue;

		auto createdUpValue = CreateObject<UpValueObject>(location);
		createdUpValue->nextUpValue = upValue;

		if (prevUpValue == nullptr)
			mOpenUpValues = createdUpValue;
		else
			prevUpValue->nextUpValue = createdUpValue;

		return createdUpValue;
	}
	void VM::ClosedUpValues(Value *end)
	{
		while (mOpenUpValues != nullptr && mOpenUpValues->location >= end)
		{
			UpValueObject *upvalue = mOpenUpValues;
			upvalue->closed = *upvalue->location;
			upvalue->location = &upvalue->closed;
			mOpenUpValues = upvalue->nextUpValue;
		}
	}

	void VM::FreeObjects()
	{
		auto bytes = mBytesAllocated;
		Object *object = mObjectChain;
		while (object != nullptr)
		{
			Object *next = object->next;
			mBytesAllocated -= sizeof(object);
			delete object;
			object = next;
		}

#ifdef GC_DEBUG
		std::wcout << "collected " << bytes - mBytesAllocated << " bytes (from " << bytes << " to " << mBytesAllocated << ") next gc bytes " << mNextGCByteSize << std::endl;
#endif
	}

	void VM::RegisterToGCRecordChain(const Value &value)
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

	void VM::GC()
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
	void VM::MarkRootObjects()
	{
		for (Value *slot = mValueStack; slot < mStackTop; ++slot)
			slot->Mark(this);
		for (int32_t i = 0; i < mFrameCount; ++i)
			mFrames[i].closure->Mark(this);
		for (UpValueObject *upvalue = mOpenUpValues; upvalue != nullptr; upvalue = upvalue->nextUpValue)
			upvalue->Mark(this);

		for (int32_t i = 0; i < GLOBAL_VARIABLE_MAX; ++i)
			if (mGlobalVariables[i] != sNullValue)
				mGlobalVariables[i].Mark(this);
	}
	void VM::MarkGrayObjects()
	{
		while (mGrayObjects.size() > 0)
		{
			auto object = mGrayObjects.back();
			mGrayObjects.pop_back();
			object->Blacken(this);
		}
	}
	void VM::Sweep()
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