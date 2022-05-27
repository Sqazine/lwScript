#include "VM.h"
#include "Library.h"
#include <ctime>
namespace lws
{
	void ExecuteRecorder::Dump()
	{
#define DUMP_OPCODE(op) \
	std::wcout << L"\t" << std::setfill(L'0') << std::setw(10) << v.callTimes << "    " << std::setfill(L'0') << std::setw(16) << v.executeTime << "	" << double(v.executeTime) / v.callTimes << "(t/c)    " << (#op) << L"\n"
		std::wcout << "call times:\n";
		for (const auto &[k, v] : mOpCodeCallTimes)
		{
			switch (k)
			{
			case OP_RETURN:
				DUMP_OPCODE(OP_RETURN);
				break;
			case OP_SAVE_TO_GLOBAL:
				DUMP_OPCODE(OP_SAVE_TO_GLOBAL);
				break;
			case OP_NEG:
				DUMP_OPCODE(OP_NEG);
				break;
			case OP_ADD:
				DUMP_OPCODE(OP_ADD);
				break;
			case OP_SUB:
				DUMP_OPCODE(OP_SUB);
				break;
			case OP_MUL:
				DUMP_OPCODE(OP_MUL);
				break;
			case OP_DIV:
				DUMP_OPCODE(OP_DIV);
				break;
			case OP_MOD:
				DUMP_OPCODE(OP_MOD);
				break;
			case OP_BIT_AND:
				DUMP_OPCODE(OP_BIT_AND);
				break;
			case OP_BIT_OR:
				DUMP_OPCODE(OP_BIT_OR);
				break;
			case OP_BIT_XOR:
				DUMP_OPCODE(OP_BIT_XOR);
				break;
			case OP_BIT_NOT:
				DUMP_OPCODE(OP_BIT_NOT);
				break;
			case OP_BIT_LEFT_SHIFT:
				DUMP_OPCODE(OP_BIT_LEFT_SHIFT);
				break;
			case OP_BIT_RIGHT_SHIFT:
				DUMP_OPCODE(OP_BIT_RIGHT_SHIFT);
				break;
			case OP_GREATER:
				DUMP_OPCODE(OP_GREATER);
				break;
			case OP_LESS:
				DUMP_OPCODE(OP_LESS);
				break;
			case OP_EQUAL:
				DUMP_OPCODE(OP_EQUAL);
				break;
			case OP_NOT:
				DUMP_OPCODE(OP_NOT);
				break;
			case OP_AND:
				DUMP_OPCODE(OP_AND);
				break;
			case OP_OR:
				DUMP_OPCODE(OP_OR);
				break;
			case OP_NEW_CONST:
				DUMP_OPCODE(OP_NEW_CONST);
				break;
			case OP_GET_VAR:
				DUMP_OPCODE(OP_GET_VAR);
				break;
			case OP_NEW_VAR:
				DUMP_OPCODE(OP_NEW_VAR);
				break;
			case OP_SET_VAR:
				DUMP_OPCODE(OP_SET_VAR);
				break;
			case OP_NEW_ARRAY:
				DUMP_OPCODE(OP_NEW_ARRAY);
				break;
			case OP_NEW_TABLE:
				DUMP_OPCODE(OP_NEW_TABLE);
				break;
			case OP_NEW_LAMBDA:
				DUMP_OPCODE(OP_NEW_LAMBDA);
				break;
			case OP_NEW_CLASS:
				DUMP_OPCODE(OP_NEW_CLASS);
				break;
			case OP_GET_INDEX_VAR:
				DUMP_OPCODE(OP_GET_INDEX_VAR);
				break;
			case OP_SET_INDEX_VAR:
				DUMP_OPCODE(OP_SET_INDEX_VAR);
				break;
			case OP_GET_CLASS_VAR:
				DUMP_OPCODE(OP_GET_CLASS_VAR);
				break;
			case OP_SET_CLASS_VAR:
				DUMP_OPCODE(OP_SET_CLASS_VAR);
				break;
			case OP_GET_CLASS_FUNCTION:
				DUMP_OPCODE(OP_GET_CLASS_FUNCTION);
				break;
			case OP_GET_FUNCTION:
				DUMP_OPCODE(OP_GET_FUNCTION);
				break;
			case OP_ENTER_SCOPE:
				DUMP_OPCODE(OP_ENTER_SCOPE);
				break;
			case OP_EXIT_SCOPE:
				DUMP_OPCODE(OP_EXIT_SCOPE);
				break;
			case OP_JUMP:
				DUMP_OPCODE(OP_JUMP);
				break;
			case OP_JUMP_IF_FALSE:
				DUMP_OPCODE(OP_JUMP_IF_FALSE);
				break;
			case OP_FUNCTION_CALL:
				DUMP_OPCODE(OP_FUNCTION_CALL);
				break;
			case OP_CONDITION:
				DUMP_OPCODE(OP_CONDITION);
				break;
			case OP_REF_VARIABLE:
				DUMP_OPCODE(OP_REF_VARIABLE);
				break;
			case OP_REF_INDEX:
				DUMP_OPCODE(OP_REF_INDEX);
				break;
			case OP_REF_OBJECT:
				DUMP_OPCODE(OP_REF_OBJECT);
				break;
			case OP_FACTORIAL:
				DUMP_OPCODE(OP_FACTORIAL);
				break;
			default:
				break;
			}
		}
#undef DUMP_OPCODE
	}

	VM::VM()
		: mContext(nullptr)
	{
		ResetStatus();
		LibraryManager::RegisterLibrary(L"IO", new IO(this));
		LibraryManager::RegisterLibrary(L"DataStructure", new DataStructure(this));
		LibraryManager::RegisterLibrary(L"Memory", new Memory(this));
		LibraryManager::RegisterLibrary(L"Time", new Time(this));
	}
	VM::~VM()
	{
		if (mContext)
		{
			delete mContext;
			mContext = nullptr;
		}
		sp = 0;
		fp = 0;
		Gc();
	}

	StrObject *VM::CreateStrObject(std::wstring_view value)
	{
		if (curObjCount == maxObjCount)
			Gc();

		StrObject *object = new StrObject(value);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	ArrayObject *VM::CreateArrayObject(const std::vector<Value> &elements)
	{
		if (curObjCount == maxObjCount)
			Gc();

		ArrayObject *object = new ArrayObject(elements);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	TableObject *VM::CreateTableObject(const ValueUnorderedMap &elements)
	{
		if (curObjCount == maxObjCount)
			Gc();

		TableObject *object = new TableObject(elements);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	ClassObject *VM::CreateClassObject(std::wstring_view name, const std::unordered_map<std::wstring, ValueDesc> &members, const std::vector<std::pair<std::wstring, ClassObject *>> &parentClasses)
	{
		if (curObjCount == maxObjCount)
			Gc();

		ClassObject *object = new ClassObject(name, members, parentClasses);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	LambdaObject *VM::CreateLambdaObject(int64_t frameIdx)
	{
		if (curObjCount == maxObjCount)
			Gc();

		LambdaObject *object = new LambdaObject(frameIdx);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	RefObject *VM::CreateRefObject(std::wstring_view name, Value index)
	{
		if (curObjCount == maxObjCount)
			Gc();

		RefObject *refObject = new RefObject(name, index);
		refObject->marked = false;

		refObject->next = firstObject;
		firstObject = refObject;

		curObjCount++;

		return refObject;
	}

	RefObject *VM::CreateRefObject(std::wstring_view address)
	{
		if (curObjCount == maxObjCount)
			Gc();

		RefObject *refObject = new RefObject(address);
		refObject->marked = false;

		refObject->next = firstObject;
		firstObject = refObject;

		curObjCount++;

		return refObject;
	}

	Value VM::Execute(Frame *frame)
	{
		PreAssemble(frame);
		Value result = ExecuteOpCode(frame);
#ifdef NEED_TO_RECORD_OPCODE_EXECUTE
		mRecorder.Dump();
#endif
		return result;
	}

	void VM::PreAssemble(Frame *frame)
	{
		for (const auto &enumframe : frame->mEnumFrames)
			ExecuteOpCode(enumframe.second);
	}

	Value VM::ExecuteOpCode(Frame *frame)
	{

#ifdef NEED_TO_RECORD_OPCODE_EXECUTE
#define START_RECORD_EXECUTE(op)                \
	mRecorder.mOpCodeCallTimes[op].callTimes++; \
	auto start = clock();

#define END_RECORD_EXECUTE(op) \
	auto end = clock();        \
	mRecorder.mOpCodeCallTimes[op].executeTime += end - start;

#else
#define START_RECORD_EXECUTE(op)
#define END_RECORD_EXECUTE(op)
#endif

		// + - * /
#define COMMON_BINARY(op)                                                                 \
	do                                                                                    \
	{                                                                                     \
		Value right = PopValue();                                                         \
		Value left = PopValue();                                                          \
		if (IS_INT_VALUE(right) && IS_INT_VALUE(left))                                    \
			PushValue(Value(TO_INT_VALUE(left) op TO_INT_VALUE(right)));                  \
		else if (IS_INT_VALUE(right) && IS_REAL_VALUE(left))                              \
			PushValue(Value(TO_REAL_VALUE(left) op TO_INT_VALUE(right)));                 \
		else if (IS_REAL_VALUE(right) && IS_INT_VALUE(left))                              \
			PushValue(Value(TO_INT_VALUE(left) op TO_REAL_VALUE(right)));                 \
		else if (IS_REAL_VALUE(right) && IS_REAL_VALUE(left))                             \
			PushValue(Value(TO_REAL_VALUE(left) op TO_REAL_VALUE(right)));                \
		else                                                                              \
			ASSERT(L"Invalid binary op:" + left.Stringify() + (L#op) + right.Stringify()) \
	} while (0);

// & | % << >>
#define INT_BINARY(op)                                                                    \
	do                                                                                    \
	{                                                                                     \
		Value right = PopValue();                                                         \
		Value left = PopValue();                                                          \
		if (IS_INT_VALUE(right) && IS_INT_VALUE(left))                                    \
			PushValue(Value(TO_INT_VALUE(left) op TO_INT_VALUE(right)));                  \
		else                                                                              \
			ASSERT(L"Invalid binary op:" + left.Stringify() + (L#op) + right.Stringify()) \
	} while (0);

// > >= < <=
#define COMPARE_BINARY(op)                                                                       \
	do                                                                                           \
	{                                                                                            \
		Value right = PopValue();                                                                \
		Value left = PopValue();                                                                 \
		if (IS_INT_VALUE(right) && IS_INT_VALUE(left))                                           \
			PushValue(TO_INT_VALUE(left) op TO_INT_VALUE(right) ? Value(true) : Value(false));   \
		else if (IS_INT_VALUE(right) && IS_REAL_VALUE(left))                                     \
			PushValue(TO_REAL_VALUE(left) op TO_INT_VALUE(right) ? Value(true) : Value(false));  \
		else if (IS_REAL_VALUE(right) && IS_INT_VALUE(left))                                     \
			PushValue(TO_INT_VALUE(left) op TO_REAL_VALUE(right) ? Value(true) : Value(false));  \
		else if (IS_REAL_VALUE(right) && IS_REAL_VALUE(left))                                    \
			PushValue(TO_REAL_VALUE(left) op TO_REAL_VALUE(right) ? Value(true) : Value(false)); \
		else                                                                                     \
			PushValue(Value(false));                                                             \
	} while (0);

//&& ||
#define LOGIC_BINARY(op)                                                                         \
	do                                                                                           \
	{                                                                                            \
		Value right = PopValue();                                                                \
		Value left = PopValue();                                                                 \
		if (IS_BOOL_VALUE(right) && IS_BOOL_VALUE(left))                                         \
			PushValue(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false)); \
		else                                                                                     \
			ASSERT(L"Invalid op:" + left.Stringify() + (L#op) + right.Stringify())               \
	} while (0);

		for (size_t ip = 0; ip < frame->mCodes.size(); ++ip)
		{
			uint64_t instruction = frame->mCodes[ip];
			switch (instruction)
			{
			case OP_RETURN:
			{
				START_RECORD_EXECUTE(OP_RETURN)
				{
					auto returnObjCount = PopValue().integer;
					if (mContext->mUpContext)
					{
						Context *tmp = mContext->GetUpContext();
						delete mContext;
						mContext = tmp;
					}

					if (returnObjCount == 1)
						return PopValue();
					else if (returnObjCount == 0)
						return Value();
				}
				END_RECORD_EXECUTE(OP_RETURN)
				break;
			}
			case OP_SAVE_TO_GLOBAL:
			{
				START_RECORD_EXECUTE(OP_SAVE_TO_GLOBAL)
				{
					auto name = frame->mStrings[frame->mCodes[++ip]];
					if (mContext->mUpContext)
					{
						Context *tmp = mContext->GetUpContext();
						delete mContext;
						mContext = tmp;
					}
					auto value = PopValue();

					auto rootContext = mContext->GetRoot();
					if (IS_INVALID_VALUE(rootContext->GetVariableByName(name)))
						rootContext->DefineVariableByName(name, ValueDescType::CONST, value);
					else
					{
						// TODO:now only for enum,to avoiding multiple assign enum class object to root context
						// here not process any date temporarily
					}
				}
				END_RECORD_EXECUTE(OP_SAVE_TO_GLOBAL);
				break;
			}
			case OP_LOAD_VALUE:
			{
				START_RECORD_EXECUTE(OP_LOAD_VALUE)
				{
					auto v = frame->mValues[frame->mCodes[++ip]];
					if(IS_STR_VALUE(v))
					{
						//add to gc module to manage its lifetime
						if (curObjCount == maxObjCount)
							Gc();

						StrObject *object = TO_STR_VALUE(v);
						object->marked = false;
						object->next = firstObject;
						firstObject = object;
						curObjCount++;
					}
					PushValue(v);
				}
				END_RECORD_EXECUTE(OP_LOAD_VALUE)
				break;
			}
			case OP_NEG:
			{
				START_RECORD_EXECUTE(OP_NEG)
				{
					Value value = PopValue();
					if (IS_REAL_VALUE(value))
						PushValue(Value(-TO_REAL_VALUE(value)));
					else if (IS_INT_VALUE(value))
						PushValue(Value(-TO_INT_VALUE(value)));
					else
						ASSERT(L"Invalid op:'-'" + value.Stringify())
				}
				END_RECORD_EXECUTE(OP_NEG)
				break;
			}
			case OP_NOT:
			{
				START_RECORD_EXECUTE(OP_NOT)
				{
					Value value = PopValue();
					if (IS_BOOL_VALUE(value))
						PushValue(Value(!TO_BOOL_VALUE(value)));
					else
						ASSERT(L"Invalid op:'!'" + value.Stringify())
				}
				END_RECORD_EXECUTE(OP_NOT)
				break;
			}
			case OP_ADD:
			{
				START_RECORD_EXECUTE(OP_ADD)
				{
					Value right = PopValue();
					Value left = PopValue();
					if (IS_INT_VALUE(right) && IS_INT_VALUE(left))
						PushValue(Value(TO_INT_VALUE(left) + TO_INT_VALUE(right)));
					else if (IS_INT_VALUE(right) && IS_REAL_VALUE(left))
						PushValue(Value(TO_REAL_VALUE(left) + TO_INT_VALUE(right)));
					else if (IS_REAL_VALUE(right) && IS_INT_VALUE(left))
						PushValue(Value(TO_INT_VALUE(left) + TO_REAL_VALUE(right)));
					else if (IS_REAL_VALUE(right) && IS_REAL_VALUE(left))
						PushValue(Value(TO_REAL_VALUE(left) + TO_REAL_VALUE(right)));
					else if (IS_STR_VALUE(right) && IS_STR_VALUE(left))
						PushValue(Value(CreateStrObject(TO_STR_VALUE(left)->value + TO_STR_VALUE(right)->value)));
					else
						ASSERT(L"Invalid binary op:" + left.Stringify() + L"+" + right.Stringify())
				}
				END_RECORD_EXECUTE(OP_ADD)
				break;
			}
			case OP_SUB:
			{
				START_RECORD_EXECUTE(OP_SUB)
				{
					COMMON_BINARY(-);
				}
				END_RECORD_EXECUTE(OP_SUB)
				break;
			}
			case OP_MUL:
			{
				START_RECORD_EXECUTE(OP_MUL)
				{
					COMMON_BINARY(*);
				}
				END_RECORD_EXECUTE(OP_MUL)
				break;
			}
			case OP_DIV:
			{
				START_RECORD_EXECUTE(OP_DIV)
				{
					COMMON_BINARY(/);
				}
				END_RECORD_EXECUTE(OP_DIV)
				break;
			}
			case OP_MOD:
			{
				START_RECORD_EXECUTE(OP_MOD)
				{
					INT_BINARY(%);
				}
				END_RECORD_EXECUTE(OP_MOD)
				break;
			}
			case OP_BIT_AND:
			{
				START_RECORD_EXECUTE(OP_BIT_AND)
				{
					INT_BINARY(&);
				}
				END_RECORD_EXECUTE(OP_BIT_AND)
				break;
			}
			case OP_BIT_OR:
			{
				START_RECORD_EXECUTE(OP_BIT_OR)
				{
					INT_BINARY(|);
				}
				END_RECORD_EXECUTE(OP_BIT_OR)
				break;
			}
			case OP_BIT_XOR:
			{
				START_RECORD_EXECUTE(OP_BIT_XOR)
				{
					INT_BINARY(^);
				}
				END_RECORD_EXECUTE(OP_BIT_OR)
				break;
			}
			case OP_BIT_NOT:
			{
				START_RECORD_EXECUTE(OP_BIT_NOT)
				{
					Value value = PopValue();
					if (IS_INT_VALUE(value))
						PushValue(Value(~TO_INT_VALUE(value)));
					else
						ASSERT(L"Invalid op:'~'" + value.Stringify())
				}
				END_RECORD_EXECUTE(OP_BIT_NOT)
				break;
			}
			case OP_BIT_LEFT_SHIFT:
			{
				START_RECORD_EXECUTE(OP_BIT_LEFT_SHIFT)
				{
					INT_BINARY(<<);
				}
				END_RECORD_EXECUTE(OP_BIT_LEFT_SHIFT)
				break;
			}
			case OP_BIT_RIGHT_SHIFT:
			{
				START_RECORD_EXECUTE(OP_BIT_RIGHT_SHIFT)
				{
					INT_BINARY(>>);
				}
				END_RECORD_EXECUTE(OP_BIT_RIGHT_SHIFT)
				break;
			}
			case OP_GREATER:
			{
				START_RECORD_EXECUTE(OP_GREATER)
				{
					COMPARE_BINARY(>);
				}
				END_RECORD_EXECUTE(OP_GREATER)
				break;
			}
			case OP_LESS:
			{
				START_RECORD_EXECUTE(OP_LESS)
				{
					COMPARE_BINARY(<);
				}

				END_RECORD_EXECUTE(OP_LESS)
				break;
			}

			case OP_AND:
			{
				START_RECORD_EXECUTE(OP_AND)
				{
					LOGIC_BINARY(&&);
				}

				END_RECORD_EXECUTE(OP_AND)
				break;
			}
			case OP_OR:
			{
				START_RECORD_EXECUTE(OP_OR)
				{
					LOGIC_BINARY(||);
				}
				END_RECORD_EXECUTE(OP_OR)
				break;
			}

			case OP_EQUAL:
			{
				START_RECORD_EXECUTE(OP_EQUAL)
				{
					Value left = PopValue();
					Value right = PopValue();
					PushValue(Value(left == right));
				}
				END_RECORD_EXECUTE(OP_EQUAL)
				break;
			}
			case OP_NEW_VAR:
			{
				START_RECORD_EXECUTE(OP_NEW_VAR)
				{
					mContext->DefineVariableByName(frame->mStrings[frame->mCodes[++ip]], ValueDescType::VARIABLE, PopValue());
				}
				END_RECORD_EXECUTE(OP_NEW_VAR)
				break;
			}
			case OP_NEW_CONST:
			{
				START_RECORD_EXECUTE(OP_NEW_CONST)
				{
					mContext->DefineVariableByName(frame->mStrings[frame->mCodes[++ip]], ValueDescType::CONST, PopValue());
				}
				END_RECORD_EXECUTE(OP_NEW_CONST)
				break;
			}
			case OP_SET_VAR:
			{
				START_RECORD_EXECUTE(OP_SET_VAR)
				{
					std::wstring name = frame->mStrings[frame->mCodes[++ip]];

					Value value = PopValue();
					Value variable = mContext->GetVariableByName(name);

					if (IS_REF_VALUE(variable) && !TO_REF_VALUE(variable)->isAddressReference)
					{
						auto refObject = TO_REF_VALUE(variable);
						if (IS_INVALID_VALUE(refObject->index))
							mContext->AssignVariableByName(refObject->name, value);
						else
						{
							variable = mContext->GetVariableByName(refObject->name);
							auto index = refObject->index;
							if (IS_ARRAY_VALUE(variable))
							{
								ArrayObject *arrayObject = TO_ARRAY_VALUE(variable);
								if (!IS_INT_VALUE(index))
									ASSERT(L"Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify())

								int64_t iIndex = TO_INT_VALUE(index);

								if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
									ASSERT(L"Index out of array range,array size:" + std::to_wstring(arrayObject->elements.size()) + L",index:" + std::to_wstring(iIndex))

								arrayObject->elements[iIndex] = value;
							}
							else if (IS_TABLE_VALUE(variable))
							{
								TableObject *tableObject = TO_TABLE_VALUE(variable);
								bool existed = false;
								for (auto [key, value] : tableObject->elements)
									if (key == index)
									{
										tableObject->elements[key] = value;
										existed = true;
										break;
									}
								if (!existed)
									tableObject->elements[index] = value;
							}
							else
								ASSERT(L"Invalid index op.The indexed object isn't a array object or a table object:" + index.Stringify())
						}
					}
					else if (IS_REF_VALUE(variable))
					{
						mContext->AssignVariableByAddress(TO_REF_VALUE(variable)->address, value);
						TO_REF_VALUE(variable)->address = PointerAddressToString(value.object); //update ref address
					}
					else
						mContext->AssignVariableByName(name, value);
				}
				END_RECORD_EXECUTE(OP_SET_VAR)
				break;
			}
			case OP_GET_VAR:
			{
				START_RECORD_EXECUTE(OP_GET_VAR)
				auto end_t = 0;
				{
					std::wstring name = frame->mStrings[frame->mCodes[++ip]];

					Value varValue = mContext->GetVariableByName(name);

					//create a class object
					if (IS_INVALID_VALUE(varValue))
					{
						if (frame->HasClassFrame(name))
						{
							end_t = clock();
							PushValue(ExecuteOpCode(frame->GetClassFrame(name)));
						}
						else
							ASSERT(L"No class or variable declaration:" + name)
					}
					else if (IS_REF_VALUE(varValue) && !TO_REF_VALUE(varValue)->isAddressReference)
					{
						auto refObject = TO_REF_VALUE(varValue);
						varValue = mContext->GetVariableByName(refObject->name);

						if (IS_INVALID_VALUE(refObject->index))
							PushValue(varValue);
						else
						{
							auto index = refObject->index;
							if (IS_ARRAY_VALUE(varValue))
							{
								ArrayObject *arrayObject = TO_ARRAY_VALUE(varValue);
								if (!IS_INT_VALUE(index))
									ASSERT(L"Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify())

								int64_t iIndex = (int64_t)TO_INT_VALUE(index);

								if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
									ASSERT(L"Index out of array range,array size:" + std::to_wstring(arrayObject->elements.size()) + L",index:" + std::to_wstring(iIndex))

								PushValue(arrayObject->elements[iIndex]);
							}
							else if (IS_TABLE_VALUE(varValue))
							{
								TableObject *tableObject = TO_TABLE_VALUE(varValue);

								bool hasValue = false;
								for (const auto [key, value] : tableObject->elements)
									if (key == index)
									{
										PushValue(value);
										hasValue = true;
										break;
									}
								if (!hasValue)
									PushValue(Value());
							}
							else
								ASSERT(L"Invalid index op.The indexed object isn't a array object or a table object:" + index.Stringify())
						}
					}
					else if (IS_REF_VALUE(varValue))
					{
						varValue = mContext->GetVariableByAddress(TO_REF_VALUE(varValue)->address);
						PushValue(varValue);
					}
					else
						PushValue(varValue);
				}
				if (end_t != 0)
					mRecorder.mOpCodeCallTimes[OP_GET_VAR].executeTime += end_t - start;
				else
				{
					END_RECORD_EXECUTE(OP_GET_VAR)
				}
				break;
			}
			case OP_NEW_ARRAY:
			{
				START_RECORD_EXECUTE(OP_NEW_ARRAY)
				{
					std::vector<Value> elements;
					int64_t arraySize = (int64_t)frame->mValues[frame->mCodes[++ip]].integer;
					for (int64_t i = 0; i < arraySize; ++i)
						elements.insert(elements.begin(), PopValue());
					PushValue(CreateArrayObject(elements));
				}
				END_RECORD_EXECUTE(OP_NEW_ARRAY)
				break;
			}
			case OP_NEW_TABLE:
			{
				START_RECORD_EXECUTE(OP_NEW_TABLE)
				{
					ValueUnorderedMap elements;
					int64_t tableSize = (int64_t)frame->mValues[frame->mCodes[++ip]].integer;
					for (int64_t i = 0; i < tableSize; ++i)
					{
						Value key = PopValue();
						Value value = PopValue();
						elements[key] = value;
					}
					PushValue(CreateTableObject(elements));
				}
				END_RECORD_EXECUTE(OP_NEW_TABLE)
				break;
			}
			case OP_NEW_CLASS:
			{
				START_RECORD_EXECUTE(OP_NEW_CLASS)
				{
					std::wstring name = frame->mStrings[frame->mCodes[++ip]];

					std::unordered_map<std::wstring, ValueDesc> members;
					std::vector<std::pair<std::wstring, ClassObject *>> parentClasses;

					for (auto value : mContext->mValues)
					{
						if (value.first.find_first_of(parentClassPrefixID) == 0 && IS_CLASS_VALUE(value.second.value))
							parentClasses.emplace_back(value.first.substr(wcslen(parentClassPrefixID)), TO_CLASS_VALUE(value.second.value));
						else
							members[value.first] = value.second;
					}

					PushValue(CreateClassObject(name, members, parentClasses));
				}
				END_RECORD_EXECUTE(OP_NEW_CLASS)
				break;
			}
			case OP_GET_INDEX_VAR:
			{
				START_RECORD_EXECUTE(OP_GET_INDEX_VAR)
				{
					Value index = PopValue();
					Value value = PopValue();
					if (IS_ARRAY_VALUE(value))
					{
						ArrayObject *arrayObject = TO_ARRAY_VALUE(value);
						if (!IS_INT_VALUE(index))
							ASSERT(L"Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify())

						int64_t iIndex = (int64_t)TO_INT_VALUE(index);

						if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
							ASSERT(L"Index out of array range,array size:" + std::to_wstring(arrayObject->elements.size()) + L",index:" + std::to_wstring(iIndex))

						PushValue(arrayObject->elements[iIndex]);
					}
					else if (IS_TABLE_VALUE(value))
					{
						TableObject *tableObject = TO_TABLE_VALUE(value);

						bool hasValue = false;
						for (const auto [key, value] : tableObject->elements)
							if (key == index)
							{
								PushValue(value);
								hasValue = true;
								break;
							}
						if (!hasValue)
							PushValue(Value());
					}
					else
						ASSERT(L"Invalid index op.The indexed object isn't a array object or a table object:" + value.Stringify())
				}
				END_RECORD_EXECUTE(OP_GET_INDEX_VAR)
				break;
			}
			case OP_SET_INDEX_VAR:
			{
				START_RECORD_EXECUTE(OP_SET_INDEX_VAR)
				{
					Value index = PopValue();
					Value value = PopValue();
					Value assigner = PopValue();

					if (IS_ARRAY_VALUE(value))
					{
						ArrayObject *arrayObject = TO_ARRAY_VALUE(value);
						if (!IS_INT_VALUE(index))
							ASSERT(L"Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify())

						int64_t iIndex = TO_INT_VALUE(index);

						if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
							ASSERT(L"Index out of array range,array size:" + std::to_wstring(arrayObject->elements.size()) + L",index:" + std::to_wstring(iIndex))

						arrayObject->elements[iIndex] = assigner;
					}
					else if (IS_TABLE_VALUE(value))
					{
						TableObject *tableObject = TO_TABLE_VALUE(value);
						bool existed = false;
						for (auto [key, value] : tableObject->elements)
							if (key == index)
							{
								tableObject->elements[key] = assigner;
								existed = true;
								break;
							}
						if (!existed)
							tableObject->elements[index] = assigner;
					}
					else
						ASSERT(L"Invalid index op.The indexed object isn't a array object or a table object:" + value.Stringify())
				}
				END_RECORD_EXECUTE(OP_SET_INDEX_VAR)
				break;
			}
			case OP_GET_CLASS_VAR:
			{
				START_RECORD_EXECUTE(OP_GET_CLASS_VAR)
				{

					std::wstring memberName = frame->mStrings[frame->mCodes[++ip]];
					Value stackTop = PopValue();
					if (!IS_CLASS_VALUE(stackTop))
						ASSERT(L"Not a class object of the callee of:" + memberName)
					ClassObject *classObj = TO_CLASS_VALUE(stackTop);
					PushValue(classObj->GetMemberByName(memberName));
				}
				END_RECORD_EXECUTE(OP_GET_CLASS_VAR)
				break;
			}
			case OP_SET_CLASS_VAR:
			{
				START_RECORD_EXECUTE(OP_SET_CLASS_VAR)
				{
					std::wstring memberName = frame->mStrings[frame->mCodes[++ip]];
					Value stackTop = PopValue();
					if (!IS_CLASS_VALUE(stackTop))
						ASSERT(L"Not a class object of the callee of:" + memberName)
					ClassObject *classObj = TO_CLASS_VALUE(stackTop);

					Value assigner = PopValue();

					classObj->AssignMemberByName(memberName, assigner);
				}
				END_RECORD_EXECUTE(OP_SET_CLASS_VAR)
				break;
			}
			case OP_GET_CLASS_FUNCTION:
			{
				START_RECORD_EXECUTE(OP_GET_CLASS_FUNCTION)
				{

					std::wstring memberName = frame->mStrings[frame->mCodes[++ip]];
					Value stackTop = PopValue();
					if (!IS_CLASS_VALUE(stackTop))
						ASSERT(L"Not a class object of the callee of:" + memberName)
					ClassObject *classObj = TO_CLASS_VALUE(stackTop);
					std::wstring classType = classObj->name;

					Frame *classFrame = nullptr;
					if (frame->HasClassFrame(classType)) //function:function add(){return 10;}
						classFrame = frame->GetClassFrame(classType);
					else
						ASSERT(L"No class declaration:" + classType)

					if (classFrame->HasFunctionFrame(memberName))
						PushFrame(classFrame->GetFunctionFrame(memberName));
					else if (!IS_INVALID_VALUE(classObj->GetMemberByName(memberName))) //lambda:let add=function(){return 10;}
					{
						Value lambdaObject = classObj->GetMemberByName(memberName);
						if (!IS_LAMBDA_VALUE(lambdaObject))
							ASSERT(L"No lambda object:" + memberName + L" in class:" + classType)
						PushFrame(classFrame->GetLambdaFrame(TO_LAMBDA_VALUE(lambdaObject)->frameIndex));
					}
					else if (!classObj->parentClasses.empty()) //get parent classs' function
					{
						for (const auto &parentClass : classObj->parentClasses)
						{
							classType = parentClass.second->name;
							classFrame = nullptr;
							if (frame->HasClassFrame(classType)) //function:function add(){return 10;}
								classFrame = frame->GetClassFrame(classType);
							else
								ASSERT(L"No class declaration:" + classType)

							if (classFrame->HasFunctionFrame(memberName))
							{
								PushFrame(classFrame->GetFunctionFrame(memberName));
								break;
							}
							else if (!IS_INVALID_VALUE(classObj->GetMemberByName(memberName))) //lambda:let add=function(){return 10;}
							{
								Value lambdaObject = classObj->GetMemberByName(memberName);
								if (!IS_LAMBDA_VALUE(lambdaObject))
									ASSERT(L"No lambda object:" + memberName + L" in class:" + classType)
								PushFrame(classFrame->GetLambdaFrame(TO_LAMBDA_VALUE(lambdaObject)->frameIndex));
								break;
							}
						}
					}
					else
						ASSERT(L"No function in class:" + memberName)
				}
				END_RECORD_EXECUTE(OP_GET_CLASS_FUNCTION)
				break;
			}
			case OP_ENTER_SCOPE:
			{
				START_RECORD_EXECUTE(OP_ENTER_SCOPE)
				{

					mContext = new Context(mContext);
				}
				END_RECORD_EXECUTE(OP_ENTER_SCOPE)
				break;
			}
			case OP_EXIT_SCOPE:
			{
				START_RECORD_EXECUTE(OP_EXIT_SCOPE)
				{
					Context *tmp = mContext->GetUpContext();
					delete mContext;
					mContext = tmp;
				}
				END_RECORD_EXECUTE(OP_EXIT_SCOPE)
				break;
			}
			case OP_JUMP_IF_FALSE:
			{
				START_RECORD_EXECUTE(OP_JUMP_IF_FALSE)
				{
					bool isJump = !TO_BOOL_VALUE(PopValue());
					uint64_t address = (uint64_t)(frame->mJumpAddresses[frame->mCodes[++ip]]);

					if (isJump)
						ip = address;
				}
				END_RECORD_EXECUTE(OP_JUMP_IF_FALSE)
				break;
			}
			case OP_JUMP:
			{
				START_RECORD_EXECUTE(OP_JUMP)
				{
					uint64_t address = (uint64_t)(frame->mJumpAddresses[frame->mCodes[++ip]]);
					ip = address;
				}
				END_RECORD_EXECUTE(OP_JUMP)
				break;
			}
			case OP_GET_FUNCTION:
			{

				START_RECORD_EXECUTE(OP_GET_FUNCTION)
				{

					std::wstring fnName = frame->mStrings[frame->mCodes[++ip]];
					if (frame->HasFunctionFrame(fnName)) //function:function add(){return 10;}
						PushFrame(frame->GetFunctionFrame(fnName));
					else if (!IS_INVALID_VALUE(mContext->GetVariableByName(fnName))) //lambda:let add=function(){return 10;}
					{
						auto lambdaObject = mContext->GetVariableByName(fnName);
						if (!IS_LAMBDA_VALUE(lambdaObject))
							ASSERT(L"Not a lambda object of " + fnName)
						PushFrame(frame->GetLambdaFrame(TO_LAMBDA_VALUE(lambdaObject)->frameIndex));
					}
					else if (HasNativeFunction(fnName))
						PushFrame(new NativeFunctionFrame(fnName));
					else
						ASSERT(L"No function:" + fnName)
				}
				END_RECORD_EXECUTE(OP_GET_FUNCTION)
				break;
			}
			case OP_FUNCTION_CALL:
			{
				START_RECORD_EXECUTE(OP_FUNCTION_CALL)
				auto end_t = 0;
				{
					auto stackTop = PopValue();

					if (IS_LAMBDA_VALUE(stackTop)) //if stack is a function object then execute it
					{
						auto argCount = TO_INT_VALUE(PopValue());
						end_t = clock();
						auto executeResult = ExecuteOpCode(frame->GetLambdaFrame(TO_LAMBDA_VALUE(stackTop)->frameIndex));
						PushValue(executeResult);
					}
					else //else execute function
					{
						auto argCount = TO_INT_VALUE(stackTop);

						if (!IsFrameStackEmpty())
						{
							Frame *f = PopFrame();
							if (IS_NATIVE_FUNCTION_FRAME(f))
							{
								std::vector<Value> args;
								for (int64_t i = 0; i < argCount; ++i)
									args.insert(args.begin(), PopValue());

								Value result = GetNativeFunction(TO_NATIVE_FUNCTION_FRAME(f)->GetName())(args);
								if (!IS_INVALID_VALUE(result))
									PushValue(result);
							}
							else
							{
								end_t = clock();
								PushValue(ExecuteOpCode(f));
							}
						}
					}
				}
				if (end_t != 0)
					mRecorder.mOpCodeCallTimes[OP_FUNCTION_CALL].executeTime += end_t - start;
				else
				{
					END_RECORD_EXECUTE(OP_FUNCTION_CALL)
				}
				break;
			}
			case OP_CONDITION:
			{
				START_RECORD_EXECUTE(OP_CONDITION)
				{

					Value condition = PopValue();
					Value trueBranch = PopValue();
					Value falseBranch = PopValue();

					if (!IS_BOOL_VALUE(condition))
						ASSERT(L"Not a bool expr of condition expr's '?'.")
					if (TO_BOOL_VALUE(condition))
						PushValue(trueBranch);
					else
						PushValue(falseBranch);
				}
				END_RECORD_EXECUTE(OP_CONDITION)
				break;
			}
			case OP_NEW_LAMBDA:
			{
				START_RECORD_EXECUTE(OP_NEW_LAMBDA)
				{
					PushValue(CreateLambdaObject(frame->mValues[frame->mCodes[++ip]].integer));
				}
				END_RECORD_EXECUTE(OP_NEW_LAMBDA)
				break;
			}
			case OP_REF_VARIABLE:
			{
				START_RECORD_EXECUTE(OP_REF_VARIABLE)
				{
					PushValue(CreateRefObject(frame->mStrings[frame->mCodes[++ip]], gInvalidValue));
				}
				END_RECORD_EXECUTE(OP_REF_VARIABLE)
				break;
			}
			case OP_REF_INDEX:
			{
				START_RECORD_EXECUTE(OP_REF_INDEX)
				{
					auto index = PopValue();
					PushValue(CreateRefObject(frame->mStrings[frame->mCodes[++ip]], index));
				}
				END_RECORD_EXECUTE(OP_REF_INDEX)
				break;
			}
			case OP_REF_OBJECT:
			{
				START_RECORD_EXECUTE(OP_REF_OBJECT)
				{
					auto value = PopValue();
					if (IS_OBJECT_VALUE(value))
						PushValue(CreateRefObject(PointerAddressToString(value.object)));
					else
						ASSERT(L"Cannot reference a value," + value.Stringify() + L"only object can be referenced,")
				}
				END_RECORD_EXECUTE(OP_REF_OBJECT)
				break;
			}
			case OP_FACTORIAL:
			{
				START_RECORD_EXECUTE(OP_FACTORIAL)
				{

					auto stackTop = PopValue();
					if (IS_INT_VALUE(stackTop))
					{
						int64_t v = Factorial(TO_INT_VALUE(stackTop));
						PushValue(Value(v));
					}
					else if (IS_REAL_VALUE(stackTop))
					{
						int64_t v = Factorial((int64_t)TO_REAL_VALUE(stackTop));
						PushValue(Value((double)v));
					}
					else
						ASSERT(L"Invalid postfix operator '!',the increment object isn't an int num object or a real num object.")
				}
				END_RECORD_EXECUTE(OP_FACTORIAL)
				break;
			}
			default:
				break;
			}
		}

		return Value();
	}

	void VM::ResetStatus()
	{
		sp = 0;
		fp = 0;
		firstObject = nullptr;
		curObjCount = 0;
		maxObjCount = GC_OBJECT_COUNT_THRESHOLD;

		curValueStackSize = VALUE_STACK_MAX;
		std::vector<Value>().swap(mValueStack);
		mValueStack.resize(curValueStackSize);

		curFrameStackSize = FRAME_STACK_MAX;
		std::vector<Frame *>().swap(mFrameStack);
		mFrameStack.resize(curFrameStackSize);

		if (mContext != nullptr)
		{
			delete mContext;
			mContext = nullptr;
		}
		mContext = new Context();
	}

	std::function<Value(std::vector<Value>)> VM::GetNativeFunction(std::wstring_view fnName)
	{
		for (const auto lib : LibraryManager::mLibraries)
			if (lib.second->HasNativeFunction(fnName))
				return lib.second->GetNativeFunction(fnName);
		return nullptr;
	}
	bool VM::HasNativeFunction(std::wstring_view name)
	{
		for (const auto lib : LibraryManager::mLibraries)
			if (lib.second->HasNativeFunction(name))
				return true;
		return false;
	}

	void VM::PushValue(Value object)
	{
		if (sp >= curValueStackSize)
		{
			auto t = mValueStack;
			curValueStackSize *= STACK_INCREMENT_RATE;
			mValueStack.resize(curValueStackSize);

			for (size_t i = 0; i < t.size(); ++i)
				mValueStack[i] = t[i];
		}
		mValueStack[sp++] = object;
	}
	Value VM::PopValue()
	{
		return mValueStack[--sp];
	}

	void VM::PushFrame(Frame *frame)
	{
		if (fp >= curFrameStackSize)
		{
			auto t = mFrameStack;
			curFrameStackSize *= STACK_INCREMENT_RATE;
			mFrameStack.resize(curValueStackSize);
			for (size_t i = 0; i < t.size(); ++i)
				mFrameStack[i] = t[i];
		}
		mFrameStack[fp++] = frame;
	}
	Frame *VM::PopFrame()
	{
		return mFrameStack[--fp];
	}

	bool VM::IsFrameStackEmpty()
	{
		return fp <= 0;
	}

	void VM::Gc()
	{
		int objNum = curObjCount;

		//mark all object which in stack;
		for (size_t i = 0; i < sp; ++i)
			mValueStack[i].Mark();
		if (mContext)
		{
			auto contextPtr = mContext;
			for (const auto &[k, v] : contextPtr->mValues)
				v.value.Mark();
			while (contextPtr->mUpContext)
			{
				contextPtr = contextPtr->mUpContext;
				for (const auto &[k, v] : contextPtr->mValues)
					v.value.Mark();
			}
		}

		//sweep objects which is not reachable
		Object **object = &firstObject;
		while (*object)
		{
			if (!((*object)->marked))
			{
				Object *unreached = *object;
				*object = unreached->next;

				delete unreached;
				unreached = nullptr;
				curObjCount--;
			}
			else
			{
				(*object)->marked = false;
				object = &(*object)->next;
			}
		}

		maxObjCount = curObjCount == 0 ? GC_OBJECT_COUNT_THRESHOLD : curObjCount * 2;

#ifdef _DEBUG
		std::wcout
			<< "Collected " << objNum - curObjCount << " objects," << curObjCount << " remaining." << std::endl;
#endif
	}
}