#include "VM.h"
#include "Library.h"
#include "Config.h"
namespace lws
{
	VM::VM()
		: mContext(nullptr)
	{
		ResetStatus();
		LibraryManager::RegisterLibrary(L"IO", new IO(this));
		LibraryManager::RegisterLibrary(L"DataStructure", new DataStructure(this));
		LibraryManager::RegisterLibrary(L"Memory", new Memory(this));
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
		return ExecuteOpCode(frame);
	}

	void VM::PreAssemble(Frame *frame)
	{
		for (const auto &enumframe : frame->mEnumFrames)
			ExecuteOpCode(enumframe.second);
	}

	Value VM::ExecuteOpCode(Frame *frame)
	{
		// + - * /
#define COMMON_BINARY(op)                                                                  \
	do                                                                                     \
	{                                                                                      \
		Value left = PopValue();                                                           \
		Value right = PopValue();                                                          \
		if (IS_INT_VALUE(right) && IS_INT_VALUE(left))                                     \
			PushValue(Value(TO_INT_VALUE(left) op TO_INT_VALUE(right)));                   \
		else if (IS_INT_VALUE(right) && IS_REAL_VALUE(left))                               \
			PushValue(Value(TO_REAL_VALUE(left) op TO_INT_VALUE(right)));                  \
		else if (IS_REAL_VALUE(right) && IS_INT_VALUE(left))                               \
			PushValue(Value(TO_INT_VALUE(left) op TO_REAL_VALUE(right)));                  \
		else if (IS_REAL_VALUE(right) && IS_REAL_VALUE(left))                              \
			PushValue(Value(TO_REAL_VALUE(left) op TO_REAL_VALUE(right)));                 \
		else                                                                               \
			Assert(L"Invalid binary op:" + left.Stringify() + (L#op) + right.Stringify()); \
	} while (0);

// & | % << >>
#define INT_BINARY(op)                                                                     \
	do                                                                                     \
	{                                                                                      \
		Value left = PopValue();                                                           \
		Value right = PopValue();                                                          \
		if (IS_INT_VALUE(right) && IS_INT_VALUE(left))                                     \
			PushValue(Value(TO_INT_VALUE(left) op TO_INT_VALUE(right)));                   \
		else                                                                               \
			Assert(L"Invalid binary op:" + left.Stringify() + (L#op) + right.Stringify()); \
	} while (0);

// > >= < <=
#define COMPARE_BINARY(op)                                                                       \
	do                                                                                           \
	{                                                                                            \
		Value left = PopValue();                                                                 \
		Value right = PopValue();                                                                \
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
		Value left = PopValue();                                                                 \
		Value right = PopValue();                                                                \
		if (IS_BOOL_VALUE(right) && IS_BOOL_VALUE(left))                                         \
			PushValue(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false)); \
		else                                                                                     \
			Assert(L"Invalid op:" + left.Stringify() + (L#op) + right.Stringify());              \
	} while (0);

		for (size_t ip = 0; ip < frame->mCodes.size(); ++ip)
		{
			uint64_t instruction = frame->mCodes[ip];
			switch (instruction)
			{
			case OP_RETURN:
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
				break;
			}
			case OP_SAVE_TO_GLOBAL:
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

				break;
			}
			case OP_NEW_REAL:
				PushValue(Value(frame->mRealNums[frame->mCodes[++ip]]));
				break;
			case OP_NEW_INT:
				PushValue(Value(frame->mIntNums[frame->mCodes[++ip]]));
				break;
			case OP_NEW_STR:
				PushValue(CreateStrObject(frame->mStrings[frame->mCodes[++ip]]));
				break;
			case OP_NEW_TRUE:
				PushValue(Value(true));
				break;
			case OP_NEW_FALSE:
				PushValue(Value(false));
				break;
			case OP_NEW_NULL:
				PushValue(Value());
				break;
			case OP_NEG:
			{
				Value value = PopValue();
				if (IS_REAL_VALUE(value))
					PushValue(Value(-TO_REAL_VALUE(value)));
				else if (IS_INT_VALUE(value))
					PushValue(Value(-TO_INT_VALUE(value)));
				else
					Assert(L"Invalid op:'-'" + value.Stringify());
				break;
			}
			case OP_NOT:
			{
				Value value = PopValue();
				if (IS_BOOL_VALUE(value))
					PushValue(Value(!TO_BOOL_VALUE(value)));
				else
					Assert(L"Invalid op:'!'" + value.Stringify());
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
				COMMON_BINARY(/);
				break;
			case OP_MOD:
				INT_BINARY(%);
				break;
			case OP_BIT_AND:
				INT_BINARY(&);
				break;
			case OP_BIT_OR:
				INT_BINARY(|);
				break;
			case OP_BIT_XOR:
				INT_BINARY(^);
				break;
			case OP_BIT_NOT:
			{
				Value value = PopValue();
				if (IS_INT_VALUE(value))
					PushValue(Value(~TO_INT_VALUE(value)));
				else
					Assert(L"Invalid op:'~'" + value.Stringify());
				break;
			}
			case OP_BIT_LEFT_SHIFT:
				INT_BINARY(<<);
				break;
			case OP_BIT_RIGHT_SHIFT:
				INT_BINARY(>>);
				break;
			case OP_GREATER:
				COMPARE_BINARY(>);
				break;
			case OP_LESS:
				COMPARE_BINARY(<);
				break;
			case OP_AND:
				LOGIC_BINARY(&&);
				break;
			case OP_OR:
				LOGIC_BINARY(||);
				break;
			case OP_EQUAL:
			{
				Value left = PopValue();
				Value right = PopValue();
				PushValue(Value(left == right));
				break;
			}
			case OP_NEW_VAR:
				mContext->DefineVariableByName(frame->mStrings[frame->mCodes[++ip]], ValueDescType::VARIABLE, PopValue());
				break;
			case OP_NEW_CONST:
				mContext->DefineVariableByName(frame->mStrings[frame->mCodes[++ip]], ValueDescType::CONST, PopValue());
				break;
			case OP_SET_VAR:
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
								Assert(L"Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());

							int64_t iIndex = TO_INT_VALUE(index);

							if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
								Assert(L"Index out of array range,array size:" + std::to_wstring(arrayObject->elements.size()) + L",index:" + std::to_wstring(iIndex));

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
							Assert(L"Invalid index op.The indexed object isn't a array object or a table object:" + index.Stringify());
					}
				}
				else if (IS_REF_VALUE(variable))
				{
					mContext->AssignVariableByAddress(TO_REF_VALUE(variable)->address, value);
					TO_REF_VALUE(variable)->address = PointerAddressToString(value.object); //update ref address
				}
				else
					mContext->AssignVariableByName(name, value);
				break;
			}
			case OP_GET_VAR:
			{
				std::wstring name = frame->mStrings[frame->mCodes[++ip]];

				Value varValue = mContext->GetVariableByName(name);

				//create a class object
				if (IS_INVALID_VALUE(varValue))
				{
					if (frame->HasClassFrame(name))
						PushValue(ExecuteOpCode(frame->GetClassFrame(name)));
					else
						Assert(L"No class or variable declaration:" + name);
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
								Assert(L"Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());

							int64_t iIndex = (int64_t)TO_INT_VALUE(index);

							if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
								Assert(L"Index out of array range,array size:" + std::to_wstring(arrayObject->elements.size()) + L",index:" + std::to_wstring(iIndex));

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
							Assert(L"Invalid index op.The indexed object isn't a array object or a table object:" + index.Stringify());
					}
				}
				else if (IS_REF_VALUE(varValue))
				{
					varValue = mContext->GetVariableByAddress(TO_REF_VALUE(varValue)->address);
					PushValue(varValue);
				}
				else
					PushValue(varValue);
				break;
			}
			case OP_NEW_ARRAY:
			{
				std::vector<Value> elements;
				int64_t arraySize = (int64_t)frame->mIntNums[frame->mCodes[++ip]];
				for (int64_t i = 0; i < arraySize; ++i)
					elements.insert(elements.begin(), PopValue());
				PushValue(CreateArrayObject(elements));
				break;
			}
			case OP_NEW_TABLE:
			{
				ValueUnorderedMap elements;
				int64_t tableSize = (int64_t)frame->mIntNums[frame->mCodes[++ip]];
				for (int64_t i = 0; i < tableSize; ++i)
				{
					Value key = PopValue();
					Value value = PopValue();
					elements[key] = value;
				}
				PushValue(CreateTableObject(elements));
				break;
			}
			case OP_NEW_CLASS:
			{
				std::wstring name = frame->mStrings[frame->mCodes[++ip]];

				std::unordered_map<std::wstring, ValueDesc> members;
				std::vector<std::pair<std::wstring, ClassObject *>> parentClasses;

				for (auto value : mContext->mValues)
				{
					if (value.first.find_first_of(containedClassPrefixID) == 0 && IS_CLASS_VALUE(value.second.value))
						parentClasses.emplace_back(value.first.substr(wcslen(containedClassPrefixID)), TO_CLASS_VALUE(value.second.value));
					else
						members[value.first] = value.second;
				}

				PushValue(CreateClassObject(name, members, parentClasses));
				break;
			}
			case OP_GET_INDEX_VAR:
			{
				Value index = PopValue();
				Value value = PopValue();
				if (IS_ARRAY_VALUE(value))
				{
					ArrayObject *arrayObject = TO_ARRAY_VALUE(value);
					if (!IS_INT_VALUE(index))
						Assert(L"Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());

					int64_t iIndex = (int64_t)TO_INT_VALUE(index);

					if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
						Assert(L"Index out of array range,array size:" + std::to_wstring(arrayObject->elements.size()) + L",index:" + std::to_wstring(iIndex));

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
					Assert(L"Invalid index op.The indexed object isn't a array object or a table object:" + value.Stringify());
				break;
			}
			case OP_SET_INDEX_VAR:
			{
				Value index = PopValue();
				Value value = PopValue();
				Value assigner = PopValue();

				if (IS_ARRAY_VALUE(value))
				{
					ArrayObject *arrayObject = TO_ARRAY_VALUE(value);
					if (!IS_INT_VALUE(index))
						Assert(L"Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());

					int64_t iIndex = TO_INT_VALUE(index);

					if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
						Assert(L"Index out of array range,array size:" + std::to_wstring(arrayObject->elements.size()) + L",index:" + std::to_wstring(iIndex));

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
					Assert(L"Invalid index op.The indexed object isn't a array object or a table object:" + value.Stringify());
				break;
			}
			case OP_GET_CLASS_VAR:
			{
				std::wstring memberName = frame->mStrings[frame->mCodes[++ip]];
				Value stackTop = PopValue();
				if (!IS_CLASS_VALUE(stackTop))
					Assert(L"Not a class object of the callee of:" + memberName);
				ClassObject *classObj = TO_CLASS_VALUE(stackTop);
				PushValue(classObj->GetMemberByName(memberName));
				break;
			}
			case OP_SET_CLASS_VAR:
			{
				std::wstring memberName = frame->mStrings[frame->mCodes[++ip]];
				Value stackTop = PopValue();
				if (!IS_CLASS_VALUE(stackTop))
					Assert(L"Not a class object of the callee of:" + memberName);
				ClassObject *classObj = TO_CLASS_VALUE(stackTop);

				Value assigner = PopValue();

				classObj->AssignMemberByName(memberName, assigner);
				break;
			}
			case OP_GET_CLASS_FUNCTION:
			{
				std::wstring memberName = frame->mStrings[frame->mCodes[++ip]];
				Value stackTop = PopValue();
				if (!IS_CLASS_VALUE(stackTop))
					Assert(L"Not a fleid object of the callee of:" + memberName);
				ClassObject *classObj = TO_CLASS_VALUE(stackTop);
				std::wstring classType = classObj->name;

				Frame *classFrame = nullptr;
				if (frame->HasClassFrame(classType)) //function:function add(){return 10;}
					classFrame = frame->GetClassFrame(classType);
				else
					Assert(L"No class declaration:" + classType);

				if (classFrame->HasFunctionFrame(memberName))
					PushFrame(classFrame->GetFunctionFrame(memberName));
				else if (!IS_INVALID_VALUE(classObj->GetMemberByName(memberName))) //lambda:let add=function(){return 10;}
				{
					Value lambdaObject = classObj->GetMemberByName(memberName);
					if (!IS_LAMBDA_VALUE(lambdaObject))
						Assert(L"No lambda object:" + memberName + L" in class:" + classType);
					PushFrame(classFrame->GetLambdaFrame(TO_LAMBDA_VALUE(lambdaObject)->frameIndex));
				}
				else if (!classObj->parentClasses.empty()) //get contained classs' function
				{
					for (const auto &containedClass : classObj->parentClasses)
					{
						classType = containedClass.second->name;
						classFrame = nullptr;
						if (frame->HasClassFrame(classType)) //function:function add(){return 10;}
							classFrame = frame->GetClassFrame(classType);
						else
							Assert(L"No class declaration:" + classType);

						if (classFrame->HasFunctionFrame(memberName))
						{
							PushFrame(classFrame->GetFunctionFrame(memberName));
							break;
						}
						else if (!IS_INVALID_VALUE(classObj->GetMemberByName(memberName))) //lambda:let add=function(){return 10;}
						{
							Value lambdaObject = classObj->GetMemberByName(memberName);
							if (!IS_LAMBDA_VALUE(lambdaObject))
								Assert(L"No lambda object:" + memberName + L" in class:" + classType);
							PushFrame(classFrame->GetLambdaFrame(TO_LAMBDA_VALUE(lambdaObject)->frameIndex));
							break;
						}
					}
				}
				else
					Assert(L"No function in class:" + memberName);
				break;
			}
			case OP_ENTER_SCOPE:
			{
				mContext = new Context(mContext);
				break;
			}
			case OP_EXIT_SCOPE:
			{
				Context *tmp = mContext->GetUpContext();
				delete mContext;
				mContext = tmp;
				break;
			}
			case OP_JUMP_IF_FALSE:
			{
				bool isJump = !TO_BOOL_VALUE(PopValue());
				uint64_t address = (uint64_t)(frame->mIntNums[frame->mCodes[++ip]]);

				if (isJump)
					ip = address;
				break;
			}
			case OP_JUMP:
			{
				uint64_t address = (uint64_t)(frame->mIntNums[frame->mCodes[++ip]]);
				ip = address;
				break;
			}
			case OP_GET_FUNCTION:
			{
				std::wstring fnName = frame->mStrings[frame->mCodes[++ip]];
				if (frame->HasFunctionFrame(fnName)) //function:function add(){return 10;}
					PushFrame(frame->GetFunctionFrame(fnName));
				else if (!IS_INVALID_VALUE(mContext->GetVariableByName(fnName))) //lambda:let add=function(){return 10;}
				{
					auto lambdaObject = mContext->GetVariableByName(fnName);
					if (!IS_LAMBDA_VALUE(lambdaObject))
						Assert(L"Not a lambda object of " + fnName);
					PushFrame(frame->GetLambdaFrame(TO_LAMBDA_VALUE(lambdaObject)->frameIndex));
				}
				else if (HasNativeFunction(fnName))
					PushFrame(new NativeFunctionFrame(fnName));
				else
					Assert(L"No function:" + fnName);
				break;
			}
			case OP_FUNCTION_CALL:
			{
				auto stackTop = PopValue();

				if (IS_LAMBDA_VALUE(stackTop)) //if stack is a function object then execute it
				{
					auto argCount = TO_INT_VALUE(PopValue());
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
							PushValue(ExecuteOpCode(f));
					}
				}
				break;
			}
			case OP_CONDITION:
			{
				Value condition = PopValue();
				Value trueBranch = PopValue();
				Value falseBranch = PopValue();

				if (!IS_BOOL_VALUE(condition))
					Assert(L"Not a bool expr of condition expr's '?'.");
				if (TO_BOOL_VALUE(condition))
					PushValue(trueBranch);
				else
					PushValue(falseBranch);
				break;
			}
			case OP_NEW_LAMBDA:
				PushValue(CreateLambdaObject(frame->mIntNums[frame->mCodes[++ip]]));
				break;
			case OP_REF_VARIABLE:
			{
				PushValue(CreateRefObject(frame->mStrings[frame->mCodes[++ip]], gInvalidValue));
				break;
			}
			case OP_REF_INDEX:
			{
				auto index = PopValue();
				PushValue(CreateRefObject(frame->mStrings[frame->mCodes[++ip]], index));
				break;
			}
			case OP_REF_OBJECT:
			{
				auto value = PopValue();
				if (IS_OBJECT_VALUE(value))
					PushValue(CreateRefObject(PointerAddressToString(value.object)));
				else
					Assert(L"Cannot reference a value," + value.Stringify() + L"only object can be referenced,");
				
				break;
			}
			case OP_SELF_INCREMENT:
			{
				auto stackTop = PopValue();
				if (IS_INT_VALUE(stackTop))
				{
					++TO_INT_VALUE(stackTop);
					PushValue(stackTop);
				}
				else if (IS_REAL_VALUE(stackTop))
				{
					++TO_REAL_VALUE(stackTop);
					PushValue(stackTop);
				}
				else
					Assert(L"Invalid prefix or postfix '++',the increment object isn't an int num object or a real num object.");
				break;
			}
			case OP_SELF_DECREMENT:
			{
				auto stackTop = PopValue();
				if (IS_INT_VALUE(stackTop))
				{
					--TO_INT_VALUE(stackTop);
					PushValue(stackTop);
				}
				else if (IS_REAL_VALUE(stackTop))
				{
					--TO_REAL_VALUE(stackTop);
					PushValue(stackTop);
				}
				else
					Assert(L"Invalid prefix or postfix operator '--',the increment object isn't an int num object or a real num object.");
				break;
			}
			case OP_FACTORIAL:
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
					Assert(L"Invalid postfix operator '!',the increment object isn't an int num object or a real num object.");
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

		std::array<Value, STACK_MAX>().swap(mValueStack);

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
		mValueStack[sp++] = object;
	}
	Value VM::PopValue()
	{
		return mValueStack[--sp];
	}

	void VM::PushFrame(Frame *frame)
	{
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