#include "VM.h"
#include <iostream>
#include "Utils.h"
#include "Object.h"
#include "Token.h"
#include "Logger.h"
namespace lwscript
{
	VM::VM()
		: mOpenUpValues(nullptr), mStackTop(nullptr), mAllocator(nullptr)
	{
		ResetStatus();
	}
	VM::~VM()
	{
		SAFE_DELETE(mAllocator);
	}

	void VM::ResetStatus()
	{
		if (mAllocator)
			SAFE_DELETE(mAllocator);

		mAllocator = new Allocator(this);

		mFrameCount = 0;
		mStackTop = mValueStack;
		mOpenUpValues = nullptr;

		memset(mGlobalVariables, 0, sizeof(Value) * GLOBAL_VARIABLE_MAX);

		for (int32_t i = 0; i < LibraryManager::GetInstance()->GetLibraries().size(); ++i)
			mGlobalVariables[i] = LibraryManager::GetInstance()->GetLibraries()[i]->Clone();
	}

	std::vector<Value> VM::Run(FunctionObject *mainFunc)
	{
		ResetStatus();

		mAllocator->RegisterToGCRecordChain(mainFunc);

		Push(mainFunc);
		auto closure = mAllocator->CreateObject<ClosureObject>(mainFunc);
		Pop();

		Push(closure);

		CallFrame *mainCallFrame = &mFrames[mFrameCount++];
		mainCallFrame->closure = closure;
		mainCallFrame->ip = closure->function->chunk.opCodes.data();
		mainCallFrame->slots = mStackTop - 1;

		Execute();

		std::vector<Value> returnValues;
#ifndef NDEBUG
		if (mStackTop != mValueStack + 1)
			Logger::Error(new Token(), L"Stack occupancy exception.");
#endif

		while (mStackTop != mValueStack + 1)
			returnValues.emplace_back(Pop());

		Pop();

		return returnValues;
	}

	void VM::Execute()
	{
		//  - * /
#define COMMON_BINARY(op)                                                                                                                                                                                    \
	do                                                                                                                                                                                                       \
	{                                                                                                                                                                                                        \
		Value right = Pop();                                                                                                                                                                                 \
		Value left = Pop();                                                                                                                                                                                  \
		if (IS_REF_VALUE(left))                                                                                                                                                                              \
			left = *TO_REF_VALUE(left)->pointer;                                                                                                                                                             \
		if (IS_REF_VALUE(right))                                                                                                                                                                             \
			right = *TO_REF_VALUE(right)->pointer;                                                                                                                                                           \
		if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                                                                                                                                                       \
			Push(TO_INT_VALUE(left) op TO_INT_VALUE(right));                                                                                                                                                 \
		else if (IS_REAL_VALUE(left) && IS_REAL_VALUE(right))                                                                                                                                                \
			Push(TO_REAL_VALUE(left) op TO_REAL_VALUE(right));                                                                                                                                               \
		else if (IS_INT_VALUE(left) && IS_REAL_VALUE(right))                                                                                                                                                 \
			Push(TO_INT_VALUE(left) op TO_REAL_VALUE(right));                                                                                                                                                \
		else if (IS_REAL_VALUE(left) && IS_INT_VALUE(right))                                                                                                                                                 \
			Push(TO_REAL_VALUE(left) op TO_INT_VALUE(right));                                                                                                                                                \
		else                                                                                                                                                                                                 \
			Logger::Error(relatedToken, L"Invalid binary op:{}{}{},only (&)int-(&)int,(&)real-(&)real,(&)int-(&)real or (&)real-(&)int type pair is available.", left.ToString(), (L#op), right.ToString()); \
	} while (0);

// & | << >>
#define INTEGER_BINARY(op)                                                                                                                                  \
	do                                                                                                                                                      \
	{                                                                                                                                                       \
		Value right = Pop();                                                                                                                                \
		Value left = Pop();                                                                                                                                 \
		if (IS_REF_VALUE(left))                                                                                                                             \
			left = *TO_REF_VALUE(left)->pointer;                                                                                                            \
		if (IS_REF_VALUE(right))                                                                                                                            \
			right = *TO_REF_VALUE(right)->pointer;                                                                                                          \
		if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                                                                                                      \
			Push(TO_INT_VALUE(left) op TO_INT_VALUE(right));                                                                                                \
		else                                                                                                                                                \
			Logger::Error(relatedToken, L"Invalid binary op:{}{}{},only (&)int-(&)int type pair is available.", left.ToString(), (L#op), right.ToString()); \
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
#define LOGIC_BINARY(op)                                                                                                                                      \
	do                                                                                                                                                        \
	{                                                                                                                                                         \
		Value right = Pop();                                                                                                                                  \
		Value left = Pop();                                                                                                                                   \
		if (IS_REF_VALUE(left))                                                                                                                               \
			left = *TO_REF_VALUE(left)->pointer;                                                                                                              \
		if (IS_REF_VALUE(right))                                                                                                                              \
			right = *TO_REF_VALUE(right)->pointer;                                                                                                            \
		if (IS_BOOL_VALUE(left) && IS_BOOL_VALUE(right))                                                                                                      \
			Push(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false));                                                                   \
		else                                                                                                                                                  \
			Logger::Error(relatedToken, L"Invalid binary op:{}{}{},only (&)bool-(&)bool type pair is available.", left.ToString(), (L#op), right.ToString()); \
	} while (0);

#define READ_INS() (*frame->ip++)

#define CHECK_IDX_RANGE(v, idx)                 \
	if (idx < 0 || idx >= (uint64_t)(v).size()) \
		Logger::Error(relatedToken, L"Idx out of range.");

#define CHECK_IDX_VALID(idxValue) \
	if (!IS_INT_VALUE(idxValue))  \
		Logger::Error(relatedToken, L"Invalid idx type for array or string,only integer is available.");

		CallFrame *frame = &mFrames[mFrameCount - 1];

		while (1)
		{
			auto instruction = READ_INS();
			auto relatedToken = frame->closure->function->chunk.opCodeRelatedTokens[READ_INS()];
			switch (instruction)
			{
			case OP_RETURN:
			{
				auto retCount = READ_INS();
				Value *retValues = mStackTop - retCount;

				ClosedUpValues(frame->slots);

				mFrameCount--;
				if (mFrameCount == 0)
					return;

				mStackTop = frame->slots;

				if (retCount == 0)
				{
					if (Config::GetInstance()->IsUseFunctionCache())
						mFrames[mFrameCount].closure->function->SetCache(mFrames[mFrameCount].arguments, {Value()});

					Push(Value());
				}
				else
				{
					if (Config::GetInstance()->IsUseFunctionCache())
					{
						std::vector<Value> rets(retValues, retValues + retCount);
						mFrames[mFrameCount].closure->function->SetCache(mFrames[mFrameCount].arguments, rets);
					}

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
				mAllocator->RegisterToGCRecordChain(vClone);
				Push(vClone);
				break;
			}
			case OP_NULL:
			{
				Push(Value());
				break;
			}
			case OP_SET_GLOBAL:
			{
				auto pos = READ_INS();
				auto v = Peek();
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
				auto value = Peek();

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
				auto v = Peek();
				*frame->closure->upvalues[pos]->location = Peek();
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
					Push(mAllocator->CreateObject<StrObject>(TO_STR_VALUE(left)->value + TO_STR_VALUE(right)->value));
				else
					Logger::Error(relatedToken, L"Invalid binary op:{}+{},only (&)int-(&)int,(&)real-(&)real,(&)int-(&)real or (&)real-(&)int type pair is available.", left.ToString(), right.ToString());
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
					Logger::Error(relatedToken, L"Invalid op:!{}, only bool type is available.", value.ToString());
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
					Logger::Error(relatedToken, L"Invalid op:-{}, only -(int||real expr) is available.", value.ToString());
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
					Logger::Error(relatedToken, L"Invalid op:{}!, only (int expr)! is available.", value.ToString());
				break;
			}
			case OP_ARRAY:
			{
				auto count = READ_INS();

				std::vector<Value> elements(count);
				size_t i = 0;
				for (auto e = mStackTop - count; e < mStackTop; ++e, ++i)
					elements[i] = *e;
				auto arrayObject = mAllocator->CreateObject<ArrayObject>(elements);

				mStackTop -= count;

				Push(arrayObject);
				break;
			}
			case OP_DICT:
			{
				auto eCount = READ_INS();
				ValueUnorderedMap elements;
				for (int64_t i = 0; i < (int64_t)eCount; ++i)
				{
					auto key = Pop();
					auto value = Pop();
					elements[key] = value;
				}
				Push(mAllocator->CreateObject<DictObject>(elements));
				break;
			}
			case OP_GET_INDEX:
			{
				auto idxValue = Pop();
				auto dsValue = Pop();
				if (IS_ARRAY_VALUE(dsValue))
				{
					auto array = TO_ARRAY_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue);

					auto intIdx = array->NormalizeIdx(TO_INT_VALUE(idxValue));
					CHECK_IDX_RANGE(array->elements, intIdx);

					Push(array->elements[intIdx]);
				}
				else if (IS_STR_VALUE(dsValue))
				{
					auto strObj = TO_STR_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue)
					auto intIdx = strObj->NormalizeIdx(TO_INT_VALUE(idxValue));
					CHECK_IDX_RANGE(strObj->value, intIdx);
					Push(mAllocator->CreateObject<StrObject>(strObj->value.substr(intIdx, 1)));
				}
				else if (IS_DICT_VALUE(dsValue))
				{
					auto dict = TO_DICT_VALUE(dsValue);

					auto iter = dict->elements.find(idxValue);

					if (iter != dict->elements.end())
						Push(iter->second);
					else
						Logger::Error(relatedToken, L"No key in dict");
				}
				break;
			}
			case OP_SET_INDEX:
			{
				auto idxValue = Pop();
				auto dsValue = Pop();
				auto newValue = Peek();
				if (IS_ARRAY_VALUE(dsValue))
				{
					auto array = TO_ARRAY_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue);
					auto intIdx = array->NormalizeIdx(TO_INT_VALUE(idxValue));
					CHECK_IDX_RANGE(array->elements, intIdx);
					array->elements[intIdx] = newValue;
				}
				else if (IS_STR_VALUE(dsValue))
				{
					auto strObj = TO_STR_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue)
					auto intIdx = strObj->NormalizeIdx(TO_INT_VALUE(idxValue));
					CHECK_IDX_RANGE(strObj->value, intIdx)

					if (!IS_STR_VALUE(newValue))
						Logger::Error(relatedToken, L"Cannot insert a non string clip:{} to string:{}", newValue.ToString(), strObj->value);

					strObj->value.append(TO_STR_VALUE(newValue)->value, intIdx, TO_STR_VALUE(newValue)->value.size());
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
				if (IsFalsey(Peek()))
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
				Push(mAllocator->CreateObject<RefObject>(&mGlobalVariables[index]));
				break;
			}
			case OP_REF_LOCAL:
			{
				auto index = READ_INS();
				Push(mAllocator->CreateObject<RefObject>(frame->slots + index));
				break;
			}
			case OP_REF_UPVALUE:
			{
				auto index = READ_INS();
				Push(mAllocator->CreateObject<RefObject>(frame->closure->upvalues[index]->location));
				break;
			}
			case OP_REF_INDEX_GLOBAL:
			{
				auto index = READ_INS();
				auto idxValue = Pop();
				if (IS_DICT_VALUE(mGlobalVariables[index]))
					Push(mAllocator->CreateObject<RefObject>(&TO_DICT_VALUE(mGlobalVariables[index])->elements[idxValue]));
				else if (IS_ARRAY_VALUE(mGlobalVariables[index]))
				{
					auto array = TO_ARRAY_VALUE(mGlobalVariables[index]);
					CHECK_IDX_VALID(idxValue)
					auto intIdx = array->NormalizeIdx(TO_INT_VALUE(idxValue));
					CHECK_IDX_RANGE(array->elements, intIdx);
					Push(mAllocator->CreateObject<RefObject>(&(array->elements[intIdx])));
				}
				else
					Logger::Error(relatedToken, L"Invalid indexed reference type:{} not a dict or array value.", mGlobalVariables[index].ToString());
				break;
			}
			case OP_REF_INDEX_LOCAL:
			{
				auto index = READ_INS();
				auto idxValue = Pop();
				Value *v = frame->slots + index;
				if (IS_DICT_VALUE((*v)))
					Push(mAllocator->CreateObject<RefObject>(&TO_DICT_VALUE((*v))->elements[idxValue]));
				else if (IS_ARRAY_VALUE((*v)))
				{
					auto array = TO_ARRAY_VALUE((*v));
					CHECK_IDX_VALID(idxValue)
					auto intIdx = array->NormalizeIdx(TO_INT_VALUE(idxValue));
					CHECK_IDX_RANGE(array->elements, intIdx);
					Push(mAllocator->CreateObject<RefObject>(&array->elements[intIdx]));
				}
				else
					Logger::Error(relatedToken, L"Invalid indexed reference type:{} not a dict or array value.", v->ToString());
				break;
			}
			case OP_REF_INDEX_UPVALUE:
			{
				auto index = READ_INS();
				auto idxValue = Pop();
				Value *v = frame->closure->upvalues[index]->location;
				if (IS_DICT_VALUE((*v)))
					Push(mAllocator->CreateObject<RefObject>(&TO_DICT_VALUE((*v))->elements[idxValue]));
				else if (IS_ARRAY_VALUE((*v)))
				{
					auto array = TO_ARRAY_VALUE((*v));
					CHECK_IDX_VALID(idxValue)
					auto intIdx = array->NormalizeIdx(TO_INT_VALUE(idxValue));
					CHECK_IDX_RANGE(array->elements, intIdx)
					Push(mAllocator->CreateObject<RefObject>(&array->elements[intIdx]));
				}
				else
					Logger::Error(relatedToken, L"Invalid indexed reference type: {}  not a dict or array value.", v->ToString());
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

					if (TO_CLOSURE_VALUE(callee)->function->varArg > VarArg::NONE)
					{
						auto arity = TO_CLOSURE_VALUE(callee)->function->arity;
						if (argCount < arity)
						{
							if (argCount == arity - 1)
							{
								if (TO_CLOSURE_VALUE(callee)->function->varArg == VarArg::WITH_NAME)
								{
									Push(new ArrayObject());
									argCount = arity;
								}
								else
									argCount = arity - 1;
							}
							else
								Logger::Error(relatedToken, L"No matching argument count.");
						}
						else if (argCount >= arity)
						{
							auto diff = argCount - arity + 1;
							if (TO_CLOSURE_VALUE(callee)->function->varArg == VarArg::WITH_NAME)
							{
								std::vector<Value> varArgs;
								for (int32_t i = 0; i < diff; ++i)
									varArgs.insert(varArgs.begin(), Pop());
								Push(new ArrayObject(varArgs));
								argCount = arity;
							}
							else
							{
								for (int32_t i = 0; i < diff; ++i)
									Pop();
								argCount = arity - 1;
							}
						}
					}
					else if (argCount != TO_CLOSURE_VALUE(callee)->function->arity)
						Logger::Error(relatedToken, L"No matching argument count.");

					std::vector<Value> args(mStackTop - argCount, mStackTop);
					std::vector<Value> rets;
					if (Config::GetInstance()->IsUseFunctionCache() && TO_CLOSURE_VALUE(callee)->function->GetCache(args, rets))
					{
						mStackTop = mStackTop - argCount - 1;
						for (int32_t i = 0; i < rets.size(); ++i)
							Push(rets[i]);
					}
					else
					{

						// init a new frame
						CallFrame *newframe = &mFrames[mFrameCount++];
						newframe->closure = TO_CLOSURE_VALUE(callee);
						newframe->ip = newframe->closure->function->chunk.opCodes.data();
						newframe->slots = mStackTop - argCount - 1;
						if (Config::GetInstance()->IsUseFunctionCache())
							newframe->arguments = args;

						frame = &mFrames[mFrameCount - 1];
					}
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
							Logger::Error(relatedToken, L"Not matching argument count of class: {}'s constructors.", klass->name);

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

					Value result;
					auto hasRetV = TO_NATIVE_FUNCTION_VALUE(callee)->fn(mStackTop - argCount, argCount, relatedToken, result);

					mStackTop -= argCount + 1;

					if (hasRetV)
						Push(result);
					else
						Push(Value());
				}
				else
					Logger::Error(relatedToken, L"Invalid callee,Only function is available: {}", callee.ToString());
				break;
			}
			case OP_CLASS:
			{
				auto name = Peek();
				auto ctorCount = READ_INS();
				auto varCount = READ_INS();
				auto constCount = READ_INS();
				auto parentClassCount = READ_INS();

				auto classObj = mAllocator->CreateObject<ClassObject>();
				classObj->name = TO_STR_VALUE(name)->value;
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
					classObj->parents[TO_STR_VALUE(name)->value] = TO_CLASS_VALUE(parentClass);
				}

				for (int32_t i = 0; i < varCount; ++i)
				{
					name = Pop();
					auto v = Pop();
					v.privilege = Privilege::MUTABLE;
					classObj->members[TO_STR_VALUE(name)->value] = v;
				}

				for (int32_t i = 0; i < constCount; ++i)
				{
					name = Pop();
					auto v = Pop();
					v.privilege = Privilege::IMMUTABLE;
					classObj->members[TO_STR_VALUE(name)->value] = v;
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
					auto key = TO_STR_VALUE(Pop())->value;
					auto value = Pop();
					elements[key] = value;
				}
				Push(mAllocator->CreateObject<AnonymousObject>(elements));
				break;
			}
			case OP_GET_PROPERTY:
			{
				auto peekValue = Peek(1);

				if (IS_REF_VALUE(peekValue))
					peekValue = *(TO_REF_VALUE(peekValue)->pointer);

				auto propName = TO_STR_VALUE(Pop())->value;
				if (IS_CLASS_VALUE(peekValue))
				{
					ClassObject *klass = TO_CLASS_VALUE(peekValue);

					Value member;
					if (klass->GetMember(propName, member))
					{
						Pop(); // pop class object
						if (IS_CLOSURE_VALUE(member))
						{
							ClassClosureBindObject *binding = mAllocator->CreateObject<ClassClosureBindObject>(klass, TO_CLOSURE_VALUE(member));
							member = Value(binding);
						}
						Push(member);
						break;
					}
					else
						Logger::Error(relatedToken, L"No member: {} in class object:{}", propName, klass->name);
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
						Logger::Error(relatedToken, L"No member: {} in enum object: {}", propName, enumObj->name);
				}
				else if (IS_ANONYMOUS_VALUE(peekValue))
				{
					auto anonymousObj = TO_ANONYMOUS_VALUE(peekValue);
					auto iter = anonymousObj->elements.find(propName);
					if (iter == anonymousObj->elements.end())
						Logger::Error(relatedToken, L"No property: {} in anonymous object:{}.", propName, anonymousObj->ToString());
					Pop(); // pop anonymouse object
					Push(iter->second);
					break;
				}
				else if (IS_MODULE_VALUE(peekValue))
				{
					auto moduleObj = TO_MODULE_VALUE(peekValue);
					Value member;
					if (moduleObj->GetMember(propName, member))
					{
						Pop(); // pop module object
						Push(member);
						break;
					}
					else
						Logger::Error(relatedToken, L"No member: {} in module: {}", propName, moduleObj->name);
				}
				else
					Logger::Error(relatedToken, L"Invalid call:not a valid class,enum or anonymous object instance: {}", peekValue.ToString());

				break;
			}
			case OP_SET_PROPERTY:
			{
				auto peekValue = Peek(1);

				if (IS_REF_VALUE(peekValue))
					peekValue = *(TO_REF_VALUE(peekValue)->pointer);

				auto propName = TO_STR_VALUE(Pop())->value;
				if (IS_CLASS_VALUE(peekValue))
				{
					auto klass = TO_CLASS_VALUE(peekValue);
					Pop(); // pop class value

					Value member;
					if (klass->GetMember(propName, member))
					{
						if (member.privilege == Privilege::IMMUTABLE)
							Logger::Error(relatedToken, L"Constant cannot be assigned twice: {}'s member: {} is a constant value", klass->name, propName);
						else
							klass->members[propName] = Peek();
					}
					else
						Logger::Error(relatedToken, L"No member named: {} in class: {}", propName, klass->name);
				}
				else if (IS_ANONYMOUS_VALUE(peekValue))
				{
					auto anonymousObj = TO_ANONYMOUS_VALUE(peekValue);
					auto iter = anonymousObj->elements.find(propName);
					if (iter == anonymousObj->elements.end())
						Logger::Error(relatedToken, L"No property: {} in anonymous object:{}", propName, anonymousObj->ToString());
					Pop(); // pop anonymouse object
					anonymousObj->elements[iter->first] = Peek();
					break;
				}
				else if (IS_ENUM_VALUE(peekValue))
					Logger::Error(relatedToken, L"Invalid call:cannot assign value to a enum object member.");
				else
					Logger::Error(relatedToken, L"Invalid call:not a valid class or anonymous object instance.");
				break;
			}
			case OP_GET_BASE:
			{
				if (!IS_CLASS_VALUE(Peek(1)))
					Logger::Error(relatedToken, L"Invalid class call:not a valid class instance.");
				auto propName = TO_STR_VALUE(Pop())->value;
				auto klass = TO_CLASS_VALUE(Pop());
				Value member;
				bool hasValue = klass->GetParentMember(propName, member);
				if (!hasValue)
					Logger::Error(relatedToken, L"No member: {} in class: {}'s parent class(es).", propName, klass->name);
				Push(member);
				break;
			}
			case OP_CLOSURE:
			{
				auto pos = READ_INS();
				auto func = TO_FUNCTION_VALUE(frame->closure->function->chunk.constants[pos]);
				mAllocator->RegisterToGCRecordChain(func);
				auto closure = mAllocator->CreateObject<ClosureObject>(func);

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
			case OP_APPREGATE_RESOLVE:
			{
				auto count = READ_INS();
				auto value = Pop();
				if (IS_ARRAY_VALUE(value))
				{
					auto arrayObj = TO_ARRAY_VALUE(value);
					if (count >= arrayObj->elements.size())
					{
						auto diff = count - arrayObj->elements.size();
						while (diff > 0)
						{
							Push(Value());
							diff--;
						}
						for (int32_t i = static_cast<int32_t>(arrayObj->elements.size() - 1); i >= 0; --i)
							Push(arrayObj->elements[i]);
					}
					else
					{
						for (int32_t i = count - 1; i >= 0; --i)
							Push(arrayObj->elements[i]);
					}
				}
				else
				{
					auto diff = count - 1;
					while (diff > 0)
					{
						Push(Value());
						diff--;
					}

					Push(value);
				}
				break;
			}
			case OP_APPREGATE_RESOLVE_VAR_ARG:
			{
				auto count = READ_INS();
				auto value = Pop();
				if (IS_ARRAY_VALUE(value))
				{
					auto arrayObj = TO_ARRAY_VALUE(value);
					if (count >= arrayObj->elements.size())
					{
						auto diff = count - arrayObj->elements.size();

						for (int32_t i = static_cast<int32_t>(diff); i > 0; --i)
						{
							if (i == diff)
								Push(mAllocator->CreateObject<ArrayObject>());
							else
								Push(Value());
						}

						for (int32_t i = static_cast<int32_t>(arrayObj->elements.size() - 1); i >= 0; --i)
							Push(arrayObj->elements[i]);
					}
					else
					{
						ArrayObject *varArgArray = mAllocator->CreateObject<ArrayObject>();
						for (int32_t i = count - 1; i < arrayObj->elements.size(); ++i)
							varArgArray->elements.emplace_back(arrayObj->elements[i]);
						Push(varArgArray);

						for (int32_t i = count - 2; i >= 0; --i)
							Push(arrayObj->elements[i]);
					}
				}
				else
				{
					auto diff = count - 2;
					while (diff > 0)
					{
						Push(Value());
						diff--;
					}

					Push(mAllocator->CreateObject<ArrayObject>());
					Push(value);
				}
				break;
			}
			case OP_MODULE:
			{
				auto name = Peek();
				auto nameStr = TO_STR_VALUE(name)->value;

				auto varCount = READ_INS();
				auto constCount = READ_INS();

				auto moduleObj = mAllocator->CreateObject<ModuleObject>();
				moduleObj->name = nameStr;
				Pop(); // pop name strobject

				for (int32_t i = 0; i < constCount; ++i)
				{
					name = Pop();
					auto v = Pop();
					v.privilege = Privilege::IMMUTABLE;
					moduleObj->values[nameStr] = v;
				}

				for (int32_t i = 0; i < varCount; ++i)
				{
					name = Pop();
					auto v = Pop();
					v.privilege = Privilege::MUTABLE;
					moduleObj->values[nameStr] = v;
				}

				Push(moduleObj);

				break;
			}
			case OP_RESET:
			{
				auto count = READ_INS();
				std::vector<Value> values(count);
				std::vector<Value> keys(count);
				for (int32_t i = count - 1; i >= 0; --i)
					keys[i] = Pop();
				for (uint32_t i = 0; i < count; ++i)
					values[i] = Pop();
				for (uint32_t i = 0; i < count; ++i)
				{
					Push(values[i]);
					Push(keys[i]);
				}
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

		auto createdUpValue = mAllocator->CreateObject<UpValueObject>(location);
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
}