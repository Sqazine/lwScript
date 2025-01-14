#include "VM.h"
#include <iostream>
#include "Allocator.h"
#include "Utils.h"
#include "Object.h"
#include "Token.h"
#include "Logger.h"
namespace lwscript
{
	std::vector<Value> VM::Run(FunctionObject *mainFunc) noexcept
	{
		PUSH_STACK(mainFunc);
		auto closure = Allocator::GetInstance()->CreateObject<ClosureObject>(mainFunc);
		POP_STACK();

		PUSH_STACK(closure);

		CallFrame mainCallFrame;
		mainCallFrame.closure = closure;
		mainCallFrame.ip = closure->function->chunk.opCodes.data();
		mainCallFrame.slots = STACK_TOP() - 1;

		PUSH_CALL_FRAME(mainCallFrame);

		Execute();

		std::vector<Value> returnValues;
#ifndef NDEBUG
		if (STACK_TOP() != STACK() + 1)
			Logger::Error(new Token(), TEXT("Stack occupancy exception."));
#endif

		while (STACK_TOP() != STACK() + 1)
			returnValues.emplace_back(POP_STACK());

		POP_STACK();

		return returnValues;
	}

	void VM::Execute()
	{
		//  - * /
#define COMMON_BINARY(op)                                                                                                                                                                                            \
	do                                                                                                                                                                                                               \
	{                                                                                                                                                                                                                \
		Value right = POP_STACK();                                                                                                                                                                                   \
		Value left = POP_STACK();                                                                                                                                                                                    \
		if (IS_REF_VALUE(left))                                                                                                                                                                                      \
			left = *TO_REF_VALUE(left)->pointer;                                                                                                                                                                     \
		if (IS_REF_VALUE(right))                                                                                                                                                                                     \
			right = *TO_REF_VALUE(right)->pointer;                                                                                                                                                                   \
		if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                                                                                                                                                               \
			PUSH_STACK(TO_INT_VALUE(left) op TO_INT_VALUE(right));                                                                                                                                                   \
		else if (IS_REAL_VALUE(left) && IS_REAL_VALUE(right))                                                                                                                                                        \
			PUSH_STACK(TO_REAL_VALUE(left) op TO_REAL_VALUE(right));                                                                                                                                                 \
		else if (IS_INT_VALUE(left) && IS_REAL_VALUE(right))                                                                                                                                                         \
			PUSH_STACK(TO_INT_VALUE(left) op TO_REAL_VALUE(right));                                                                                                                                                  \
		else if (IS_REAL_VALUE(left) && IS_INT_VALUE(right))                                                                                                                                                         \
			PUSH_STACK(TO_REAL_VALUE(left) op TO_INT_VALUE(right));                                                                                                                                                  \
		else                                                                                                                                                                                                         \
			Logger::Error(relatedToken, TEXT("Invalid binary op:{}{}{},only (&)int-(&)int,(&)real-(&)real,(&)int-(&)real or (&)real-(&)int type pair is available."), left.ToString(), TEXT(#op), right.ToString()); \
	} while (0);

// & | << >>
#define INTEGER_BINARY(op)                                                                                                                                          \
	do                                                                                                                                                              \
	{                                                                                                                                                               \
		Value right = POP_STACK();                                                                                                                                  \
		Value left = POP_STACK();                                                                                                                                   \
		if (IS_REF_VALUE(left))                                                                                                                                     \
			left = *TO_REF_VALUE(left)->pointer;                                                                                                                    \
		if (IS_REF_VALUE(right))                                                                                                                                    \
			right = *TO_REF_VALUE(right)->pointer;                                                                                                                  \
		if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                                                                                                              \
			PUSH_STACK(TO_INT_VALUE(left) op TO_INT_VALUE(right));                                                                                                  \
		else                                                                                                                                                        \
			Logger::Error(relatedToken, TEXT("Invalid binary op:{}{}{},only (&)int-(&)int type pair is available."), left.ToString(), TEXT(#op), right.ToString()); \
	} while (0);

// > <
#define COMPARE_BINARY(op)                                                          \
	do                                                                              \
	{                                                                               \
		Value right = POP_STACK();                                                  \
		Value left = POP_STACK();                                                   \
		if (IS_REF_VALUE(left))                                                     \
			left = *TO_REF_VALUE(left)->pointer;                                    \
		if (IS_REF_VALUE(right))                                                    \
			right = *TO_REF_VALUE(right)->pointer;                                  \
		if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                              \
			PUSH_STACK(TO_INT_VALUE(left) op TO_INT_VALUE(right) ? true : false);   \
		else if (IS_REAL_VALUE(left) && IS_REAL_VALUE(right))                       \
			PUSH_STACK(TO_REAL_VALUE(left) op TO_REAL_VALUE(right) ? true : false); \
		else if (IS_INT_VALUE(left) && IS_REAL_VALUE(right))                        \
			PUSH_STACK(TO_INT_VALUE(left) op TO_REAL_VALUE(right) ? true : false);  \
		else if (IS_REAL_VALUE(left) && IS_INT_VALUE(right))                        \
			PUSH_STACK(TO_REAL_VALUE(left) op TO_INT_VALUE(right) ? true : false);  \
		else                                                                        \
			PUSH_STACK(false);                                                      \
	} while (0);

// && ||
#define LOGIC_BINARY(op)                                                                                                                                              \
	do                                                                                                                                                                \
	{                                                                                                                                                                 \
		Value right = POP_STACK();                                                                                                                                    \
		Value left = POP_STACK();                                                                                                                                     \
		if (IS_REF_VALUE(left))                                                                                                                                       \
			left = *TO_REF_VALUE(left)->pointer;                                                                                                                      \
		if (IS_REF_VALUE(right))                                                                                                                                      \
			right = *TO_REF_VALUE(right)->pointer;                                                                                                                    \
		if (IS_BOOL_VALUE(left) && IS_BOOL_VALUE(right))                                                                                                              \
			PUSH_STACK(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false));                                                                     \
		else                                                                                                                                                          \
			Logger::Error(relatedToken, TEXT("Invalid binary op:{}{}{},only (&)bool-(&)bool type pair is available."), left.ToString(), TEXT(#op), right.ToString()); \
	} while (0);

#define READ_INS() (*frame->ip++)

#define CHECK_IDX_RANGE(v, idx)                 \
	if (idx < 0 || idx >= (uint64_t)(v).size()) \
		Logger::Error(relatedToken, TEXT("Idx out of range."));

#define CHECK_IDX_VALID(idxValue) \
	if (!IS_INT_VALUE(idxValue))  \
		Logger::Error(relatedToken, TEXT("Invalid idx type for array or string,only integer is available."));

		while (1)
		{
			if (IS_CALL_FRAME_STACK_EMPTY())
				return;
			CallFrame *frame = PEEK_CALL_FRAME(0);

			auto instruction = READ_INS();
			auto relatedToken = frame->closure->function->chunk.opCodeRelatedTokens[READ_INS()];
			switch (instruction)
			{
			case OP_RETURN:
			{
				auto retCount = READ_INS();
				Value *retValues = STACK_TOP() - retCount;

				CLOSED_UPVALUES(frame->slots);

				if (IS_CALL_FRAME_STACK_EMPTY())
					return;

				SET_STACK_TOP(frame->slots);

				if (retCount == 0)
				{
#ifdef LWSCRIPT_FUNCTION_CACHE_OPT
					{
						auto callFrameTop = PEEK_CALL_FRAME(0);
						callFrameTop->closure->function->SetCache(callFrameTop->argumentsHash, {Value()});
					}
#endif

					PUSH_STACK(Value());
				}
				else
				{
#ifdef LWSCRIPT_FUNCTION_CACHE_OPT
					{
						auto callFrameTop = PEEK_CALL_FRAME(0);
						std::vector<Value> rets(retValues, retValues + retCount);
						callFrameTop->closure->function->SetCache(callFrameTop->argumentsHash, rets);
					}
#endif

					uint8_t i = 0;
					while (i < retCount)
					{
						auto value = *(retValues + i);
						PUSH_STACK(value);
						i++;
					}
				}

				frame = POP_CALL_FRAME();
				break;
			}
			case OP_CONSTANT:
			{
				auto pos = READ_INS();
				auto v = frame->closure->function->chunk.constants[pos];
				PUSH_STACK(v);
				break;
			}
			case OP_NULL:
			{
				PUSH_STACK(Value());
				break;
			}
			case OP_SET_GLOBAL:
			{
				auto pos = READ_INS();
				auto v = PEEK_STACK(0);

				auto globalValue = GET_GLOBAL_VARIABLE(pos);

				if (IS_REF_VALUE(*globalValue))
					*TO_REF_VALUE(*globalValue)->pointer = v;
				else
					*globalValue = v;
				break;
			}
			case OP_GET_GLOBAL:
			{
				auto pos = READ_INS();
				PUSH_STACK(*GET_GLOBAL_VARIABLE(pos));
				break;
			}
			case OP_SET_LOCAL:
			{
				auto pos = READ_INS();
				auto value = PEEK_STACK(0);

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
				PUSH_STACK(frame->slots[pos]); // now assume base ptr on the stack bottom
				break;
			}
			case OP_SET_UPVALUE:
			{
				auto pos = READ_INS();
				auto v = PEEK_STACK(0);
				*frame->closure->upvalues[pos]->location = PEEK_STACK(0);
				break;
			}
			case OP_GET_UPVALUE:
			{
				auto pos = READ_INS();
				PUSH_STACK(*frame->closure->upvalues[pos]->location);
				break;
			}
			case OP_CLOSE_UPVALUE:
			{
				CLOSED_UPVALUES(STACK_TOP() - 1);
				POP_STACK();
				break;
			}
			case OP_ADD:
			{
				Value left = PEEK_STACK(0);
				Value right = PEEK_STACK(1);
				Value result;
				if (IS_REF_VALUE(left))
					left = *TO_REF_VALUE(left)->pointer;
				if (IS_REF_VALUE(right))
					right = *TO_REF_VALUE(right)->pointer;
				if (IS_INT_VALUE(left) && IS_INT_VALUE(right))
					result = TO_INT_VALUE(left) + TO_INT_VALUE(right);
				else if (IS_REAL_VALUE(left) && IS_REAL_VALUE(right))
					result = TO_REAL_VALUE(left) + TO_REAL_VALUE(right);
				else if (IS_INT_VALUE(left) && IS_REAL_VALUE(right))
					result = TO_INT_VALUE(left) + TO_REAL_VALUE(right);
				else if (IS_REAL_VALUE(left) && IS_INT_VALUE(right))
					result = TO_REAL_VALUE(left) + TO_INT_VALUE(right);
				else if (IS_STR_VALUE(left) && IS_STR_VALUE(right))
					result = Allocator::GetInstance()->CreateObject<StrObject>(TO_STR_VALUE(left)->value + TO_STR_VALUE(right)->value);
				else
					Logger::Error(relatedToken, TEXT("Invalid binary op:{}+{},only (&)int-(&)int,(&)real-(&)real,(&)int-(&)real or (&)real-(&)int type pair is available."), left.ToString(), right.ToString());

				MOVE_STACK_TOP(-2);
				PUSH_STACK(result);

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
				auto value = POP_STACK();
				if (IS_REF_VALUE(value))
					value = *TO_REF_VALUE(value)->pointer;
				if (!IS_BOOL_VALUE(value))
					Logger::Error(relatedToken, TEXT("Invalid op:!{}, only bool type is available."), value.ToString());
				PUSH_STACK(!TO_BOOL_VALUE(value));
				break;
			}
			case OP_EQUAL:
			{
				Value left = POP_STACK();
				Value right = POP_STACK();
				if (IS_REF_VALUE(left))
					left = *TO_REF_VALUE(left)->pointer;
				if (IS_REF_VALUE(right))
					right = *TO_REF_VALUE(right)->pointer;
				PUSH_STACK(left == right);
				break;
			}
			case OP_MINUS:
			{
				auto value = POP_STACK();
				if (IS_REF_VALUE(value))
					value = *TO_REF_VALUE(value)->pointer;
				if (IS_INT_VALUE(value))
					PUSH_STACK(-TO_INT_VALUE(value));
				else if (IS_REAL_VALUE(value))
					PUSH_STACK(-TO_REAL_VALUE(value));
				else
					Logger::Error(relatedToken, TEXT("Invalid op:-{}, only -(int||real expr) is available."), value.ToString());
				break;
			}
			case OP_FACTORIAL:
			{
				auto value = POP_STACK();
				if (IS_REF_VALUE(value))
					value = *TO_REF_VALUE(value)->pointer;
				if (IS_INT_VALUE(value))
					PUSH_STACK(Factorial(TO_INT_VALUE(value)));
				else
					Logger::Error(relatedToken, TEXT("Invalid op:{}!, only (int expr)! is available."), value.ToString());
				break;
			}
			case OP_ARRAY:
			{
				auto count = READ_INS();

				std::vector<Value> elements(count);
				size_t i = 0;
				for (auto e = STACK_TOP() - count; e < STACK_TOP(); ++e, ++i)
					elements[i] = *e;

				auto arrayObject = Allocator::GetInstance()->CreateObject<ArrayObject>(elements);

				MOVE_STACK_TOP(-count);

				PUSH_STACK(arrayObject);
				break;
			}
			case OP_DICT:
			{
				auto count = READ_INS();
				ValueUnorderedMap elements;

				auto dict = Allocator::GetInstance()->CreateObject<DictObject>(elements);

				for (auto e = STACK_TOP() - count * 2; e < STACK_TOP(); e += 2)
				{
					auto key = *e;
					auto value = *(e + 1);
					dict->elements[key] = value;
				}

				MOVE_STACK_TOP(-count * 2);

				PUSH_STACK(dict);
				break;
			}
			case OP_GET_INDEX:
			{
				auto idxValue = POP_STACK();
				auto dsValue = POP_STACK();
				if (IS_ARRAY_VALUE(dsValue))
				{
					auto array = TO_ARRAY_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue);

					auto intIdx = NormalizeIdx(TO_INT_VALUE(idxValue), array->elements.size());
					CHECK_IDX_RANGE(array->elements, intIdx);

					PUSH_STACK(array->elements[intIdx]);
				}
				else if (IS_STR_VALUE(dsValue))
				{
					auto strObj = TO_STR_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue)
					auto intIdx = NormalizeIdx(TO_INT_VALUE(idxValue), strObj->value.size());
					CHECK_IDX_RANGE(strObj->value, intIdx);
					PUSH_STACK(Allocator::GetInstance()->CreateObject<StrObject>(strObj->value.substr(intIdx, 1)));
				}
				else if (IS_DICT_VALUE(dsValue))
				{
					auto dict = TO_DICT_VALUE(dsValue);

					auto iter = dict->elements.find(idxValue);

					if (iter != dict->elements.end())
						PUSH_STACK(iter->second);
					else
						Logger::Error(relatedToken, TEXT("No key in dict"));
				}
				break;
			}
			case OP_SET_INDEX:
			{
				auto idxValue = POP_STACK();
				auto dsValue = POP_STACK();
				auto newValue = PEEK_STACK(0);
				if (IS_ARRAY_VALUE(dsValue))
				{
					auto array = TO_ARRAY_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue);
					auto intIdx = NormalizeIdx(TO_INT_VALUE(idxValue), array->elements.size());
					CHECK_IDX_RANGE(array->elements, intIdx);
					array->elements[intIdx] = newValue;
				}
				else if (IS_STR_VALUE(dsValue))
				{
					auto strObj = TO_STR_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue)
					auto intIdx = NormalizeIdx(TO_INT_VALUE(idxValue), strObj->value.size());
					CHECK_IDX_RANGE(strObj->value, intIdx)

					if (!IS_STR_VALUE(newValue))
						Logger::Error(relatedToken, TEXT("Cannot insert a non string clip:{} to string:{}"), newValue.ToString(), strObj->value);

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
				POP_STACK();
				break;
			}
			case OP_JUMP_IF_FALSE:
			{
				uint16_t address = (*(frame->ip++) << 8) | (*(frame->ip++));
				if (IsFalsey(PEEK_STACK(0)))
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
				PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(GET_GLOBAL_VARIABLE(index)));
				break;
			}
			case OP_REF_LOCAL:
			{
				auto index = READ_INS();
				PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(frame->slots + index));
				break;
			}
			case OP_REF_UPVALUE:
			{
				auto index = READ_INS();
				PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(frame->closure->upvalues[index]->location));
				break;
			}
			case OP_REF_INDEX_GLOBAL:
			{
				auto index = READ_INS();
				auto idxValue = POP_STACK();

				auto globalValue = GET_GLOBAL_VARIABLE(index);

				if (IS_DICT_VALUE(*globalValue))
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&TO_DICT_VALUE(*globalValue)->elements[idxValue]));
				else if (IS_ARRAY_VALUE(*globalValue))
				{
					auto array = TO_ARRAY_VALUE(*globalValue);
					CHECK_IDX_VALID(idxValue)
					auto intIdx = NormalizeIdx(TO_INT_VALUE(idxValue), array->elements.size());
					CHECK_IDX_RANGE(array->elements, intIdx);
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&(array->elements[intIdx])));
				}
				else
					Logger::Error(relatedToken, TEXT("Invalid indexed reference type:{} not a dict or array value."), globalValue->ToString());
				break;
			}
			case OP_REF_INDEX_LOCAL:
			{
				auto index = READ_INS();
				auto idxValue = POP_STACK();
				Value *v = frame->slots + index;
				if (IS_DICT_VALUE((*v)))
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&TO_DICT_VALUE((*v))->elements[idxValue]));
				else if (IS_ARRAY_VALUE((*v)))
				{
					auto array = TO_ARRAY_VALUE((*v));
					CHECK_IDX_VALID(idxValue)
					auto intIdx = NormalizeIdx(TO_INT_VALUE(idxValue), array->elements.size());
					CHECK_IDX_RANGE(array->elements, intIdx);
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&array->elements[intIdx]));
				}
				else
					Logger::Error(relatedToken, TEXT("Invalid indexed reference type:{} not a dict or array value."), v->ToString());
				break;
			}
			case OP_REF_INDEX_UPVALUE:
			{
				auto index = READ_INS();
				auto idxValue = POP_STACK();
				Value *v = frame->closure->upvalues[index]->location;
				if (IS_DICT_VALUE((*v)))
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&TO_DICT_VALUE((*v))->elements[idxValue]));
				else if (IS_ARRAY_VALUE((*v)))
				{
					auto array = TO_ARRAY_VALUE((*v));
					CHECK_IDX_VALID(idxValue)
					auto intIdx = NormalizeIdx(TO_INT_VALUE(idxValue), array->elements.size());
					CHECK_IDX_RANGE(array->elements, intIdx)
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&array->elements[intIdx]));
				}
				else
					Logger::Error(relatedToken, TEXT("Invalid indexed reference type: {}  not a dict or array value."), v->ToString());
				break;
			}
			case OP_CALL:
			{
				auto argCount = READ_INS();
				auto callee = PEEK_STACK(argCount);
				if (IS_CLOSURE_VALUE(callee) || IS_CLASS_CLOSURE_BIND_VALUE(callee)) // normal function or class member function
				{
					if (IS_CLASS_CLOSURE_BIND_VALUE(callee))
					{
						auto binding = TO_CLASS_CLOSURE_BIND_VALUE(callee);

						SET_VALUE_FROM_STACK_TOP_OFFSET(-(argCount + 1), binding->receiver);
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
									PUSH_STACK(new ArrayObject());
									argCount = arity;
								}
								else
									argCount = arity - 1;
							}
							else
								Logger::Error(relatedToken, TEXT("No matching argument count."));
						}
						else if (argCount >= arity)
						{
							auto diff = argCount - arity + 1;
							if (TO_CLOSURE_VALUE(callee)->function->varArg == VarArg::WITH_NAME)
							{
								std::vector<Value> varArgs;
								for (int32_t i = 0; i < diff; ++i)
									varArgs.insert(varArgs.begin(), POP_STACK());
								PUSH_STACK(new ArrayObject(varArgs));
								argCount = arity;
							}
							else
							{
								for (int32_t i = 0; i < diff; ++i)
									POP_STACK();
								argCount = arity - 1;
							}
						}
					}
					else if (argCount != TO_CLOSURE_VALUE(callee)->function->arity)
						Logger::Error(relatedToken, TEXT("No matching argument count."));

					auto argsHash = HashValueList(STACK_TOP() - argCount, STACK_TOP());
					std::vector<Value> rets;
#ifdef LWSCRIPT_FUNCTION_CACHE_OPT
					if (TO_CLOSURE_VALUE(callee)->function->GetCache(argsHash, rets))
					{
						MOVE_STACK_TOP(-(argCount + 1));
						for (int32_t i = 0; i < rets.size(); ++i)
							PUSH_STACK(rets[i]);
					}
					else
#endif
					{
						// init a new frame
						CallFrame newframe;
						newframe.closure = TO_CLOSURE_VALUE(callee);
						newframe.ip = newframe.closure->function->chunk.opCodes.data();
						newframe.slots = STACK_TOP() - argCount - 1;
#ifdef LWSCRIPT_FUNCTION_CACHE_OPT
						newframe.argumentsHash = argsHash;
#endif
						PUSH_CALL_FRAME(newframe);
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
							Logger::Error(relatedToken, TEXT("Not matching argument count of class: {}'s constructors."), klass->name);

						auto ctor = iter->second;
						// init a new frame
						CallFrame newframe;
						newframe.closure = ctor;
						newframe.ip = newframe.closure->function->chunk.opCodes.data();
						newframe.slots = STACK_TOP() - argCount - 1;

						PUSH_CALL_FRAME(newframe);
					}
				}
				else if (IS_NATIVE_FUNCTION_VALUE(callee)) // native function
				{

					Value result;
					auto hasRetV = TO_NATIVE_FUNCTION_VALUE(callee)->fn(STACK_TOP() - argCount, argCount, relatedToken, result);

					MOVE_STACK_TOP(-(argCount + 1));

					if (hasRetV)
						PUSH_STACK(result);
					else
						PUSH_STACK(Value());
				}
				else
					Logger::Error(relatedToken, TEXT("Invalid callee,Only function is available: {}"), callee.ToString());
				break;
			}
			case OP_CLASS:
			{
				auto name = PEEK_STACK(0);
				auto ctorCount = READ_INS();
				auto varCount = READ_INS();
				auto constCount = READ_INS();
				auto parentClassCount = READ_INS();

				auto classObj = Allocator::GetInstance()->CreateObject<ClassObject>();

				classObj->name = TO_STR_VALUE(name)->value;
				POP_STACK(); // pop name strobject

				for (int32_t i = 0; i < ctorCount; ++i)
				{
					auto v = TO_CLOSURE_VALUE(POP_STACK());
					classObj->constructors[v->function->arity] = v;
				}

				for (int32_t i = 0; i < parentClassCount; ++i)
				{
					name = POP_STACK();
					auto parentClass = POP_STACK();
					classObj->parents[TO_STR_VALUE(name)->value] = TO_CLASS_VALUE(parentClass);
				}

				for (int32_t i = 0; i < varCount; ++i)
				{
					name = POP_STACK();
					auto v = POP_STACK();
					v.privilege = Privilege::MUTABLE;
					classObj->members[TO_STR_VALUE(name)->value] = v;
				}

				for (int32_t i = 0; i < constCount; ++i)
				{
					name = POP_STACK();
					auto v = POP_STACK();
					v.privilege = Privilege::IMMUTABLE;
					classObj->members[TO_STR_VALUE(name)->value] = v;
				}

				PUSH_STACK(classObj);
				break;
			}
			case OP_STRUCT:
			{
				auto eCount = READ_INS();
				auto structObj = Allocator::GetInstance()->CreateObject<StructObject>();
				for (int64_t i = 0; i < (int64_t)eCount; ++i)
				{
					auto key = TO_STR_VALUE(POP_STACK())->value;
					auto value = POP_STACK();
					structObj->elements[key] = value;
				}
				PUSH_STACK(structObj);
				break;
			}
			case OP_GET_PROPERTY:
			{
				auto peekValue = PEEK_STACK(1);

				if (IS_REF_VALUE(peekValue))
					peekValue = *(TO_REF_VALUE(peekValue)->pointer);

				auto propName = TO_STR_VALUE(POP_STACK())->value;
				if (IS_CLASS_VALUE(peekValue))
				{
					ClassObject *klass = TO_CLASS_VALUE(peekValue);

					Value member;
					if (klass->GetMember(propName, member))
					{
						POP_STACK(); // pop class object
						if (IS_CLOSURE_VALUE(member))
							member = Allocator::GetInstance()->CreateObject<ClassClosureBindObject>(klass, TO_CLOSURE_VALUE(member));

						PUSH_STACK(member);
						break;
					}
					else
						Logger::Error(relatedToken, TEXT("No member: {} in class object:{}"), propName, klass->name);
				}
				else if (IS_ENUM_VALUE(peekValue))
				{
					EnumObject *enumObj = TO_ENUM_VALUE(peekValue);

					Value member;
					if (enumObj->GetMember(propName, member))
					{
						POP_STACK(); // pop enum object
						PUSH_STACK(member);
						break;
					}
					else
						Logger::Error(relatedToken, TEXT("No member: {} in enum object: {}"), propName, enumObj->name);
				}
				else if (IS_STRUCT_VALUE(peekValue))
				{
					auto structObj = TO_STRUCT_VALUE(peekValue);
					auto iter = structObj->elements.find(propName);
					if (iter == structObj->elements.end())
						Logger::Error(relatedToken, TEXT("No property: {} in struct object:{}."), propName, structObj->ToString());
					POP_STACK(); // pop struct object
					PUSH_STACK(iter->second);
					break;
				}
				else if (IS_MODULE_VALUE(peekValue))
				{
					auto moduleObj = TO_MODULE_VALUE(peekValue);
					Value member;
					if (moduleObj->GetMember(propName, member))
					{
						POP_STACK(); // pop module object
						PUSH_STACK(member);
						break;
					}
					else
						Logger::Error(relatedToken, TEXT("No member: {} in module: {}"), propName, moduleObj->name);
				}
				else
					Logger::Error(relatedToken, TEXT("Invalid call:not a valid class,enum or struct object instance: {}"), peekValue.ToString());

				break;
			}
			case OP_SET_PROPERTY:
			{
				auto peekValue = PEEK_STACK(1);

				if (IS_REF_VALUE(peekValue))
					peekValue = *(TO_REF_VALUE(peekValue)->pointer);

				auto propName = TO_STR_VALUE(POP_STACK())->value;
				if (IS_CLASS_VALUE(peekValue))
				{
					auto klass = TO_CLASS_VALUE(peekValue);
					POP_STACK(); // pop class value

					Value member;
					if (klass->GetMember(propName, member))
					{
						if (member.privilege == Privilege::IMMUTABLE)
							Logger::Error(relatedToken, TEXT("Constant cannot be assigned twice: {}'s member: {} is a constant value"), klass->name, propName);
						else
							klass->members[propName] = PEEK_STACK(0);
					}
					else
						Logger::Error(relatedToken, TEXT("No member named: {} in class: {}"), propName, klass->name);
				}
				else if (IS_STRUCT_VALUE(peekValue))
				{
					auto structObj = TO_STRUCT_VALUE(peekValue);
					auto iter = structObj->elements.find(propName);
					if (iter == structObj->elements.end())
						Logger::Error(relatedToken, TEXT("No property: {} in struct object:{}"), propName, structObj->ToString());
					POP_STACK(); // pop struct object
					structObj->elements[iter->first] = PEEK_STACK(0);
					break;
				}
				else if (IS_ENUM_VALUE(peekValue))
					Logger::Error(relatedToken, TEXT("Invalid call:cannot assign value to a enum object member."));
				else
					Logger::Error(relatedToken, TEXT("Invalid call:not a valid class or struct object instance."));
				break;
			}
			case OP_GET_BASE:
			{
				if (!IS_CLASS_VALUE(PEEK_STACK(1)))
					Logger::Error(relatedToken, TEXT("Invalid class call:not a valid class instance."));
				auto propName = TO_STR_VALUE(POP_STACK())->value;
				auto klass = TO_CLASS_VALUE(POP_STACK());
				Value member;
				bool hasValue = klass->GetParentMember(propName, member);
				if (!hasValue)
					Logger::Error(relatedToken, TEXT("No member: {} in class: {}'s parent class(es)."), propName, klass->name);
				PUSH_STACK(member);
				break;
			}
			case OP_CLOSURE:
			{
				auto pos = READ_INS();
				auto func = TO_FUNCTION_VALUE(frame->closure->function->chunk.constants[pos]);

				PUSH_STACK(func); // push function object for avoiding gc
				auto closure = Allocator::GetInstance()->CreateObject<ClosureObject>(func);
				POP_STACK(); // pop function object

				PUSH_STACK(closure);

				for (int32_t i = 0; i < closure->upvalues.size(); ++i)
				{
					auto index = READ_INS();
					auto depth = READ_INS();
					if (depth == CALL_FRAME_COUNT() - 1)
					{
						auto captured = CAPTURE_UPVALUE(frame->slots + index);
						closure->upvalues[i] = captured;
					}
					else
						closure->upvalues[i] = frame->closure->upvalues[index];
				}

				break;
			}
			case OP_APPREGATE_RESOLVE:
			{
				auto count = READ_INS();
				auto value = POP_STACK();
				if (IS_ARRAY_VALUE(value))
				{
					auto arrayObj = TO_ARRAY_VALUE(value);
					if (count >= arrayObj->elements.size())
					{
						auto diff = count - arrayObj->elements.size();
						while (diff > 0)
						{
							PUSH_STACK(Value());
							diff--;
						}
						for (int32_t i = static_cast<int32_t>(arrayObj->elements.size() - 1); i >= 0; --i)
							PUSH_STACK(arrayObj->elements[i]);
					}
					else
					{
						for (int32_t i = count - 1; i >= 0; --i)
							PUSH_STACK(arrayObj->elements[i]);
					}
				}
				else
				{
					auto diff = count - 1;
					while (diff > 0)
					{
						PUSH_STACK(Value());
						diff--;
					}

					PUSH_STACK(value);
				}
				break;
			}
			case OP_APPREGATE_RESOLVE_VAR_ARG:
			{
				auto count = READ_INS();
				auto value = PEEK_STACK(0);
				if (IS_ARRAY_VALUE(value))
				{
					auto arrayObj = TO_ARRAY_VALUE(value);
					if (count >= arrayObj->elements.size())
					{
						ArrayObject *varArgArray = Allocator::GetInstance()->CreateObject<ArrayObject>();

						POP_STACK(); // pop value object

						auto diff = count - arrayObj->elements.size();
						for (int32_t i = static_cast<int32_t>(diff); i > 0; --i)
						{
							if (i == diff)
								PUSH_STACK(varArgArray);
							else
								PUSH_STACK(Value());
						}

						for (int32_t i = static_cast<int32_t>(arrayObj->elements.size() - 1); i >= 0; --i)
							PUSH_STACK(arrayObj->elements[i]);
					}
					else
					{
						ArrayObject *varArgArray = Allocator::GetInstance()->CreateObject<ArrayObject>();

						POP_STACK(); // pop value object

						for (int32_t i = count - 1; i < arrayObj->elements.size(); ++i)
							varArgArray->elements.emplace_back(arrayObj->elements[i]);
						PUSH_STACK(varArgArray);

						for (int32_t i = count - 2; i >= 0; --i)
							PUSH_STACK(arrayObj->elements[i]);
					}
				}
				else
				{
					auto arrayObj = Allocator::GetInstance()->CreateObject<ArrayObject>();

					POP_STACK(); // pop value object

					auto diff = count - 2;
					while (diff > 0)
					{
						PUSH_STACK(Value());
						diff--;
					}

					PUSH_STACK(arrayObj);
					PUSH_STACK(value);
				}
				break;
			}
			case OP_MODULE:
			{
				auto name = PEEK_STACK(0);
				auto nameStr = TO_STR_VALUE(name)->value;

				auto varCount = READ_INS();
				auto constCount = READ_INS();

				auto moduleObj = Allocator::GetInstance()->CreateObject<ModuleObject>();
				moduleObj->name = nameStr;
				POP_STACK(); // pop name strobject

				for (int32_t i = 0; i < constCount; ++i)
				{
					name = POP_STACK();
					nameStr = TO_STR_VALUE(name)->value;
					auto v = POP_STACK();
					v.privilege = Privilege::IMMUTABLE;
					moduleObj->values[nameStr] = v;
				}

				for (int32_t i = 0; i < varCount; ++i)
				{
					name = POP_STACK();
					nameStr = TO_STR_VALUE(name)->value;
					auto v = POP_STACK();
					v.privilege = Privilege::MUTABLE;
					moduleObj->values[nameStr] = v;
				}

				PUSH_STACK(moduleObj);

				break;
			}
			case OP_RESET:
			{
				auto count = READ_INS();
				std::vector<Value> values(count);
				std::vector<Value> keys(count);
				for (int32_t i = count - 1; i >= 0; --i)
					keys[i] = POP_STACK();
				for (uint32_t i = 0; i < count; ++i)
					values[i] = POP_STACK();
				for (uint32_t i = 0; i < count; ++i)
				{
					PUSH_STACK(values[i]);
					PUSH_STACK(keys[i]);
				}
				break;
			}
			default:
				break;
			}
		}
	}

	bool VM::IsFalsey(const Value &v) noexcept
	{
		return IS_NULL_VALUE(v) || (IS_BOOL_VALUE(v) && !TO_BOOL_VALUE(v));
	}
}