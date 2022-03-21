#include "VM.h"
#include "Library.h"
#include "Config.h"
namespace lws
{
	VM::VM()
		: mContext(nullptr)
	{
		ResetStatus();
		LibraryManager::RegisterLibrary("IO", new IO(this));
		LibraryManager::RegisterLibrary("DataStructure", new DataStructure(this));
		LibraryManager::RegisterLibrary("Memory", new Memory(this));
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

	RealNumObject *VM::CreateRealNumObject(double value)
	{
		if (curObjCount == maxObjCount)
			Gc();

		RealNumObject *object = new RealNumObject(value);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	IntNumObject *VM::CreateIntNumObject(int64_t value)
	{
		if (curObjCount == maxObjCount)
			Gc();

		IntNumObject *object = new IntNumObject(value);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	StrObject *VM::CreateStrObject(std::string_view value)
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
	BoolObject *VM::CreateBoolObject(bool value)
	{
		if (curObjCount == maxObjCount)
			Gc();

		BoolObject *object = new BoolObject(value);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	NullObject *VM::CreateNullObject()
	{
		if (curObjCount == maxObjCount)
			Gc();

		NullObject *object = new NullObject();
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}
	ArrayObject *VM::CreateArrayObject(const std::vector<Object *> &elements)
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

	TableObject *VM::CreateTableObject(const std::unordered_map<Object *, Object *> &elements)
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

	FieldObject *VM::CreateFieldObject(std::string_view name, const std::unordered_map<std::string, Object *> &members, const std::vector<std::pair<std::string, FieldObject *>> &containedFields)
	{
		if (curObjCount == maxObjCount)
			Gc();

		FieldObject *object = new FieldObject(name, members, containedFields);
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

	RefObject *VM::CreateRefObject(std::string_view name, Object *index)
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

	Object *VM::Execute(Frame *frame)
	{
// + - * /
#define COMMON_BINARY(op)                                                                           \
	do                                                                                              \
	{                                                                                               \
		Object *left = PopObject();                                                                 \
		Object *right = PopObject();                                                                \
		if (IS_INT_OBJ(right) && IS_INT_OBJ(left))                                                  \
			PushObject(CreateIntNumObject(TO_INT_OBJ(left)->value op TO_INT_OBJ(right)->value));    \
		else if (IS_INT_OBJ(right) && IS_REAL_OBJ(left))                                            \
			PushObject(CreateRealNumObject(TO_REAL_OBJ(left)->value op TO_INT_OBJ(right)->value));  \
		else if (IS_REAL_OBJ(right) && IS_INT_OBJ(left))                                            \
			PushObject(CreateRealNumObject(TO_INT_OBJ(left)->value op TO_REAL_OBJ(right)->value));  \
		else if (IS_REAL_OBJ(right) && IS_REAL_OBJ(left))                                           \
			PushObject(CreateRealNumObject(TO_REAL_OBJ(left)->value op TO_REAL_OBJ(right)->value)); \
		else                                                                                        \
			Assert("Invalid binary op:" + left->Stringify() + (#op) + right->Stringify());          \
	} while (0);

// & | % << >>
#define INT_BINARY(op)                                                                           \
	do                                                                                           \
	{                                                                                            \
		Object *left = PopObject();                                                              \
		Object *right = PopObject();                                                             \
		if (IS_INT_OBJ(right) && IS_INT_OBJ(left))                                               \
			PushObject(CreateIntNumObject(TO_INT_OBJ(left)->value op TO_INT_OBJ(right)->value)); \
		else                                                                                     \
			Assert("Invalid binary op:" + left->Stringify() + (#op) + right->Stringify());       \
	} while (0);

// > >= < <=
#define COMPARE_BINARY(op)                                                                                                        \
	do                                                                                                                            \
	{                                                                                                                             \
		Object *left = PopObject();                                                                                               \
		Object *right = PopObject();                                                                                              \
		if (IS_INT_OBJ(right) && IS_INT_OBJ(left))                                                                                \
			PushObject(TO_INT_OBJ(left)->value op TO_INT_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));   \
		else if (IS_INT_OBJ(right) && IS_REAL_OBJ(left))                                                                          \
			PushObject(TO_REAL_OBJ(left)->value op TO_INT_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));  \
		else if (IS_REAL_OBJ(right) && IS_INT_OBJ(left))                                                                          \
			PushObject(TO_INT_OBJ(left)->value op TO_REAL_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));  \
		else if (IS_REAL_OBJ(right) && IS_REAL_OBJ(left))                                                                         \
			PushObject(TO_REAL_OBJ(left)->value op TO_REAL_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false)); \
		else                                                                                                                      \
			PushObject(CreateBoolObject(false));                                                                                  \
	} while (0);

//&& ||
#define LOGIC_BINARY(op)                                                                                                               \
	do                                                                                                                                 \
	{                                                                                                                                  \
		Object *left = PopObject();                                                                                                    \
		Object *right = PopObject();                                                                                                   \
		if (IS_BOOL_OBJ(right) && IS_BOOL_OBJ(left))                                                                                   \
			PushObject(((BoolObject *)left)->value op((BoolObject *)right)->value ? CreateBoolObject(true) : CreateBoolObject(false)); \
		else                                                                                                                           \
			Assert("Invalid op:" + left->Stringify() + (#op) + right->Stringify());                                                    \
	} while (0);

		for (size_t ip = 0; ip < frame->mCodes.size(); ++ip)
		{
			uint64_t instruction = frame->mCodes[ip];
			switch (instruction)
			{
			case OP_RETURN:
				if (mContext->mUpContext)
				{
					Context *tmp = mContext->GetUpContext();
					delete mContext;
					mContext = tmp;
				}
				return PopObject();
				break;
			case OP_NEW_REAL:
				PushObject(CreateRealNumObject(frame->mRealNums[frame->mCodes[++ip]]));
				break;
			case OP_NEW_INT:
				PushObject(CreateIntNumObject(frame->mIntNums[frame->mCodes[++ip]]));
				break;
			case OP_NEW_STR:
				PushObject(CreateStrObject(frame->mStrings[frame->mCodes[++ip]]));
				break;
			case OP_NEW_TRUE:
				PushObject(CreateBoolObject(true));
				break;
			case OP_NEW_FALSE:
				PushObject(CreateBoolObject(false));
				break;
			case OP_NEW_NULL:
				PushObject(CreateNullObject());
				break;
			case OP_NEG:
			{
				Object *object = PopObject();
				if (IS_REAL_OBJ(object))
					PushObject(CreateRealNumObject(-TO_REAL_OBJ(object)->value));
				else if (IS_INT_OBJ(object))
					PushObject(CreateIntNumObject(-TO_INT_OBJ(object)->value));
				else
					Assert("Invalid op:'-'" + object->Stringify());
				break;
			}
			case OP_NOT:
			{
				Object *object = PopObject();
				if (IS_BOOL_OBJ(object))
					PushObject(CreateBoolObject(!TO_BOOL_OBJ(object)->value));
				else
					Assert("Invalid op:'!'" + object->Stringify());
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
				Object *object = PopObject();
				if (IS_INT_OBJ(object))
					PushObject(CreateIntNumObject(~TO_INT_OBJ(object)->value));
				else
					Assert("Invalid op:'~'" + object->Stringify());
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
				Object *left = PopObject();
				Object *right = PopObject();
				PushObject(CreateBoolObject(left->IsEqualTo(right)));
				break;
			}
			case OP_NEW_VAR:
			{
				Object *value = PopObject();
				mContext->DefineVariableByName(frame->mStrings[frame->mCodes[++ip]], ObjectDescType::VARIABLE, value);
				break;
			}
			case OP_NEW_CONST:
			{
				Object *value = PopObject();
				mContext->DefineVariableByName(frame->mStrings[frame->mCodes[++ip]], ObjectDescType::CONST, value);
				break;
			}
			case OP_SET_VAR:
			{
				std::string name = frame->mStrings[frame->mCodes[++ip]];

				Object *value = PopObject();
				Object *variable = mContext->GetVariableByName(name);

				if (IS_REF_OBJ(variable))
				{
					auto refObject = TO_REF_OBJ(variable);
					if (refObject->index == nullptr)
						mContext->AssignVariableByName(refObject->name, value);
					else
					{
						variable = mContext->GetVariableByName(refObject->name);
						auto index = refObject->index;
						if (IS_ARRAY_OBJ(variable))
						{
							ArrayObject *arrayObject = TO_ARRAY_OBJ(variable);
							if (!IS_INT_OBJ(index))
								Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index->Stringify());

							int64_t iIndex = TO_INT_OBJ(index)->value;

							if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
								Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

							arrayObject->elements[iIndex] = value;
						}
						else if (IS_TABLE_OBJ(variable))
						{
							TableObject *tableObject = TO_TABLE_OBJ(variable);
							bool existed = false;
							for (auto [key, value] : tableObject->elements)
								if (key->IsEqualTo(index))
								{
									tableObject->elements[key] = value;
									existed = true;
									break;
								}
							if (!existed)
								tableObject->elements[index] = value;
						}
						else
							Assert("Invalid index op.The indexed object isn't a array object or a table object:" + index->Stringify());
					}
				}
				else
					mContext->AssignVariableByName(name, value);
				break;
			}
			case OP_GET_VAR:
			{
				std::string name = frame->mStrings[frame->mCodes[++ip]];

				Object *varObject = mContext->GetVariableByName(name);

				//create a field object
				if (varObject == nullptr)
				{
					if (frame->HasFieldFrame(name))
						PushObject(Execute(frame->GetFieldFrame(name)));
					else
						Assert("No field declaration:" + name);
				}
				else if (IS_REF_OBJ(varObject))
				{
					auto refObject = TO_REF_OBJ(varObject);
					varObject = mContext->GetVariableByName(refObject->name);

					if (refObject->index == nullptr)
						PushObject(varObject);
					else
					{
						auto index = refObject->index;
						if (IS_ARRAY_OBJ(varObject))
						{
							ArrayObject *arrayObject = TO_ARRAY_OBJ(varObject);
							if (!IS_INT_OBJ(index))
								Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index->Stringify());

							int64_t iIndex = (int64_t)TO_INT_OBJ(index)->value;

							if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
								Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

							PushObject(arrayObject->elements[iIndex]);
						}
						else if (IS_TABLE_OBJ(varObject))
						{
							TableObject *tableObject = TO_TABLE_OBJ(varObject);

							bool hasValue = false;
							for (const auto [key, value] : tableObject->elements)
								if (key->IsEqualTo(index))
								{
									PushObject(value);
									hasValue = true;
									break;
								}
							if (!hasValue)
								PushObject(CreateNullObject());
						}
						else
							Assert("Invalid index op.The indexed object isn't a array object or a table object:" + index->Stringify());
					}
				}
				else
					PushObject(varObject);
				break;
			}
			case OP_NEW_ARRAY:
			{
				std::vector<Object *> elements;
				int64_t arraySize = (int64_t)frame->mIntNums[frame->mCodes[++ip]];
				for (int64_t i = 0; i < arraySize; ++i)
					elements.insert(elements.begin(), PopObject());
				PushObject(CreateArrayObject(elements));
				break;
			}
			case OP_NEW_TABLE:
			{
				std::unordered_map<Object *, Object *> elements;
				int64_t tableSize = (int64_t)frame->mIntNums[frame->mCodes[++ip]];
				for (int64_t i = 0; i < tableSize; ++i)
				{
					Object *key = PopObject();
					Object *value = PopObject();
					elements[key] = value;
				}
				PushObject(CreateTableObject(elements));
				break;
			}
			case OP_NEW_FIELD:
			{
				std::string name = frame->mStrings[frame->mCodes[++ip]];

				std::unordered_map<std::string, Object *> members;
				std::vector<std::pair<std::string, FieldObject *>> containedFields;

				for (auto value : mContext->mValues)
				{
					if (value.first.find_first_of(containedFieldPrefixID) == 0 && IS_FIELD_OBJ(value.second.object))
						containedFields.emplace_back(std::make_pair(value.first.substr(containedFieldPrefixID.size()), TO_FIELD_OBJ(value.second.object)));
					else
						members[value.first] = value.second.object;
				}

				PushObject(CreateFieldObject(name, members, containedFields));
				break;
			}
			case OP_GET_INDEX_VAR:
			{
				Object *index = PopObject();
				Object *object = PopObject();
				if (IS_ARRAY_OBJ(object))
				{
					ArrayObject *arrayObject = TO_ARRAY_OBJ(object);
					if (!IS_INT_OBJ(index))
						Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index->Stringify());

					int64_t iIndex = (int64_t)TO_INT_OBJ(index)->value;

					if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
						Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

					PushObject(arrayObject->elements[iIndex]);
				}
				else if (IS_TABLE_OBJ(object))
				{
					TableObject *tableObject = TO_TABLE_OBJ(object);

					bool hasValue = false;
					for (const auto [key, value] : tableObject->elements)
						if (key->IsEqualTo(index))
						{
							PushObject(value);
							hasValue = true;
							break;
						}
					if (!hasValue)
						PushObject(CreateNullObject());
				}
				else
					Assert("Invalid index op.The indexed object isn't a array object or a table object:" + object->Stringify());
				break;
			}
			case OP_SET_INDEX_VAR:
			{
				Object *index = PopObject();
				Object *object = PopObject();
				Object *assigner = PopObject();

				if (IS_ARRAY_OBJ(object))
				{
					ArrayObject *arrayObject = TO_ARRAY_OBJ(object);
					if (!IS_INT_OBJ(index))
						Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index->Stringify());

					int64_t iIndex = TO_INT_OBJ(index)->value;

					if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
						Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

					arrayObject->elements[iIndex] = assigner;
				}
				else if (IS_TABLE_OBJ(object))
				{
					TableObject *tableObject = TO_TABLE_OBJ(object);
					bool existed = false;
					for (auto [key, value] : tableObject->elements)
						if (key->IsEqualTo(index))
						{
							tableObject->elements[key] = assigner;
							existed = true;
							break;
						}
					if (!existed)
						tableObject->elements[index] = assigner;
				}
				else
					Assert("Invalid index op.The indexed object isn't a array object or a table object:" + object->Stringify());
				break;
			}
			case OP_GET_FIELD_VAR:
			{
				std::string memberName = frame->mStrings[frame->mCodes[++ip]];
				Object *stackTop = PopObject();
				if (!IS_FIELD_OBJ(stackTop))
					Assert("Not a field object of the callee of:" + memberName);
				FieldObject *fieldObj = TO_FIELD_OBJ(stackTop);
				PushObject(fieldObj->GetMemberByName(memberName));
				break;
			}
			case OP_SET_FIELD_VAR:
			{
				std::string memberName = frame->mStrings[frame->mCodes[++ip]];
				Object *stackTop = PopObject();
				if (!IS_FIELD_OBJ(stackTop))
					Assert("Not a field object of the callee of:" + memberName);
				FieldObject *fieldObj = TO_FIELD_OBJ(stackTop);

				Object *assigner = PopObject();

				fieldObj->AssignMemberByName(memberName, assigner);
				break;
			}
			case OP_GET_FIELD_FUNCTION:
			{
				std::string memberName = frame->mStrings[frame->mCodes[++ip]];
				Object *stackTop = PopObject();
				if (!IS_FIELD_OBJ(stackTop))
					Assert("Not a fleid object of the callee of:" + memberName);
				FieldObject *fieldObj = TO_FIELD_OBJ(stackTop);
				std::string fieldType = fieldObj->name;

				Frame *fieldFrame = nullptr;
				if (frame->HasFieldFrame(fieldType)) //function:function add(){return 10;}
					fieldFrame = frame->GetFieldFrame(fieldType);
				else
					Assert("No field declaration:" + fieldType);

				if (fieldFrame->HasFunctionFrame(memberName))
					PushFrame(fieldFrame->GetFunctionFrame(memberName));
				else if (fieldObj->GetMemberByName(memberName) != nullptr) //lambda:let add=function(){return 10;}
				{
					Object *lambdaObject = fieldObj->GetMemberByName(memberName);
					if (!IS_LAMBDA_OBJ(lambdaObject))
						Assert("No lambda object:" + memberName + " in field:" + fieldType);
					PushFrame(fieldFrame->GetLambdaFrame(TO_LAMBDA_OBJ(lambdaObject)->frameIndex));
				}
				else if (!fieldObj->containedFields.empty()) //get contained fields' function
				{
					for (const auto &containedField : fieldObj->containedFields)
					{
						fieldType = containedField.second->name;
						fieldFrame = nullptr;
						if (frame->HasFieldFrame(fieldType)) //function:function add(){return 10;}
							fieldFrame = frame->GetFieldFrame(fieldType);
						else
							Assert("No field declaration:" + fieldType);

						if (fieldFrame->HasFunctionFrame(memberName))
						{
							PushFrame(fieldFrame->GetFunctionFrame(memberName));
							break;
						}
						else if (fieldObj->GetMemberByName(memberName) != nullptr) //lambda:let add=function(){return 10;}
						{
							Object *lambdaObject = fieldObj->GetMemberByName(memberName);
							if (!IS_LAMBDA_OBJ(lambdaObject))
								Assert("No lambda object:" + memberName + " in field:" + fieldType);
							PushFrame(fieldFrame->GetLambdaFrame(TO_LAMBDA_OBJ(lambdaObject)->frameIndex));
							break;
						}
					}
				}
				else
					Assert("No function in field:" + memberName);
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
				bool isJump = !TO_BOOL_OBJ(PopObject())->value;
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
				std::string fnName = frame->mStrings[frame->mCodes[++ip]];
				if (frame->HasFunctionFrame(fnName)) //function:function add(){return 10;}
					PushFrame(frame->GetFunctionFrame(fnName));
				else if (mContext->GetVariableByName(fnName) != nullptr) //lambda:let add=function(){return 10;}
				{
					Object *lambdaObject = mContext->GetVariableByName(fnName);
					if (!IS_LAMBDA_OBJ(lambdaObject))
						Assert("Not a lambda object of " + fnName);
					PushFrame(frame->GetLambdaFrame(TO_LAMBDA_OBJ(lambdaObject)->frameIndex));
				}
				else if (HasNativeFunction(fnName))
					PushFrame(new NativeFunctionFrame(fnName));
				else
					Assert("No function:" + fnName);
				break;
			}
			case OP_FUNCTION_CALL:
			{
				Object *stackTop = PopObject();

				if (IS_LAMBDA_OBJ(stackTop)) //if stack is a function object then execute it
				{
					IntNumObject *argCount = TO_INT_OBJ(PopObject());
					Object *executeResult = Execute(frame->GetLambdaFrame(TO_LAMBDA_OBJ(stackTop)->frameIndex));
					PushObject(executeResult);
				}
				else //else execute function
				{
					IntNumObject *argCount = TO_INT_OBJ(stackTop);

					if (!IsFrameStackEmpty())
					{
						Frame *f = PopFrame();
						if (IS_NATIVE_FUNCTION_FRAME(f))
						{
							std::vector<Object *> args;
							for (int64_t i = 0; i < argCount->value; ++i)
								args.insert(args.begin(), PopObject());

							Object *result = GetNativeFunction(TO_NATIVE_FUNCTION_FRAME(f)->GetName())(args);
							if (result)
								PushObject(result);
						}
						else
							PushObject(Execute(f));
					}
				}
				break;
			}
			case OP_CONDITION:
			{
				Object *condition = PopObject();
				Object *trueBranch = PopObject();
				Object *falseBranch = PopObject();

				if (!IS_BOOL_OBJ(condition))
					Assert("Not a bool expr of condition expr's '?'.");
				if (TO_BOOL_OBJ(condition)->value)
					PushObject(trueBranch);
				else
					PushObject(falseBranch);
				break;
			}
			case OP_NEW_LAMBDA:
				PushObject(CreateLambdaObject(frame->mIntNums[frame->mCodes[++ip]]));
				break;
			case OP_REF_VARIABLE:
			{
				PushObject(CreateRefObject(frame->mStrings[frame->mCodes[++ip]]));
				break;
			}
			case OP_REF_INDEX:
			{
				auto index = PopObject();
				PushObject(CreateRefObject(frame->mStrings[frame->mCodes[++ip]], index));
				break;
			}
			default:
				break;
			}
		}

		return CreateNullObject();
	}

	void VM::ResetStatus()
	{
		sp = 0;
		fp = 0;
		firstObject = nullptr;
		curObjCount = 0;
		maxObjCount = INIT_OBJ_NUM_MAX;

		std::array<Object *, STACK_MAX>().swap(mObjectStack);

		if (mContext != nullptr)
		{
			delete mContext;
			mContext = nullptr;
		}
		mContext = new Context();
	}

	std::function<Object *(std::vector<Object *>)> VM::GetNativeFunction(std::string_view fnName)
	{
		for (const auto lib : LibraryManager::mLibraries)
			if (lib.second->HasNativeFunction(fnName))
				return lib.second->GetNativeFunction(fnName);
		return nullptr;
	}
	bool VM::HasNativeFunction(std::string_view name)
	{
		for (const auto lib : LibraryManager::mLibraries)
			if (lib.second->HasNativeFunction(name))
				return true;
		return false;
	}

	void VM::PushObject(Object *object)
	{
		mObjectStack[sp++] = object;
	}
	Object *VM::PopObject()
	{
		return mObjectStack[--sp];
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
			mObjectStack[i]->Mark();

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

#ifdef _DEBUG
		std::cout << "Collected " << objNum - curObjCount << " objects," << curObjCount << " remaining." << std::endl;
#endif
	}
}