#include "VM.h"
namespace lws
{
	VM::VM()
		: m_Context(nullptr)
	{
		ResetStatus();
		m_Libraries["IO"] = new IO(this);
		m_Libraries["DataStructure"] = new DataStructure(this);
	}
	VM::~VM()
	{
		if (m_Context)
		{
			delete m_Context;
			m_Context = nullptr;
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

	NilObject *VM::CreateNilObject()
	{
		if (curObjCount == maxObjCount)
			Gc();

		NilObject *object = new NilObject();
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

	ClassObject *VM::CreateClassObject(std::string_view name, const std::unordered_map<std::string, Object *> &pubMembers,
									   const std::unordered_map<std::string, Object *> &proMembers,
									   const std::unordered_map<std::string, Object *> &priMembers)
	{
		if (curObjCount == maxObjCount)
			Gc();

		ClassObject *object = new ClassObject(name, pubMembers,proMembers,priMembers);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	FunctionObject *VM::CreateFunctionObject(int64_t frameIdx)
	{
		if (curObjCount == maxObjCount)
			Gc();

		FunctionObject *object = new FunctionObject(frameIdx);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	RefObject *VM::CreateRefObject(std::string_view address)
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

	Object *VM::Execute(Frame *frame)
	{
		// + - * /
#define COMMON_BINARY(op)                                                                                    \
	do                                                                                                       \
	{                                                                                                        \
		Object *left = PopObject();                                                                          \
		Object *right = PopObject();                                                                         \
		if (IS_INTEGER_OBJ(right) && IS_INTEGER_OBJ(left))                                                   \
			PushObject(CreateIntNumObject(TO_INTEGER_OBJ(left)->value op TO_INTEGER_OBJ(right)->value));    \
		else if (IS_INTEGER_OBJ(right) && IS_FLOATING_OBJ(left))                                             \
			PushObject(CreateRealNumObject(TO_FLOATING_OBJ(left)->value op TO_INTEGER_OBJ(right)->value));  \
		else if (IS_FLOATING_OBJ(right) && IS_INTEGER_OBJ(left))                                             \
			PushObject(CreateRealNumObject(TO_INTEGER_OBJ(left)->value op TO_FLOATING_OBJ(right)->value));  \
		else if (IS_FLOATING_OBJ(right) && IS_FLOATING_OBJ(left))                                            \
			PushObject(CreateRealNumObject(TO_FLOATING_OBJ(left)->value op TO_FLOATING_OBJ(right)->value)); \
		else                                                                                                 \
			Assert("Invalid binary op:" + left->Stringify() + (#op) + right->Stringify());                   \
	} while (0);
// & | % << >>
#define INTEGER_BINARY(op)                                                                                \
	do                                                                                                    \
	{                                                                                                     \
		Object *left = PopObject();                                                                       \
		Object *right = PopObject();                                                                      \
		if (IS_INTEGER_OBJ(right) && IS_INTEGER_OBJ(left))                                                \
			PushObject(CreateIntNumObject(TO_INTEGER_OBJ(left)->value op TO_INTEGER_OBJ(right)->value)); \
		else                                                                                              \
			Assert("Invalid binary op:" + left->Stringify() + (#op) + right->Stringify());                \
	} while (0);

// > >= < <= == !=
#define COMPARE_BINARY(op)                                                                                                                \
	do                                                                                                                                    \
	{                                                                                                                                     \
		Object *left = PopObject();                                                                                                       \
		Object *right = PopObject();                                                                                                      \
		if (IS_INTEGER_OBJ(right) && IS_INTEGER_OBJ(left))                                                                                \
			PushObject(TO_INTEGER_OBJ(left)->value op TO_INTEGER_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));   \
		else if (IS_INTEGER_OBJ(right) && IS_FLOATING_OBJ(left))                                                                          \
			PushObject(TO_FLOATING_OBJ(left)->value op TO_INTEGER_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));  \
		else if (IS_FLOATING_OBJ(right) && IS_INTEGER_OBJ(left))                                                                          \
			PushObject(TO_INTEGER_OBJ(left)->value op TO_FLOATING_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));  \
		else if (IS_FLOATING_OBJ(right) && IS_FLOATING_OBJ(left))                                                                         \
			PushObject(TO_FLOATING_OBJ(left)->value op TO_FLOATING_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false)); \
		else if (IS_BOOL_OBJ(right) && IS_BOOL_OBJ(left))                                                                                 \
			PushObject(TO_BOOL_OBJ(left)->value op TO_BOOL_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));         \
		else if (IS_NIL_OBJ(right) && IS_NIL_OBJ(left))                                                                                   \
			PushObject(TO_NIL_OBJ(left) op TO_NIL_OBJ(right) ? CreateBoolObject(true) : CreateBoolObject(false));                         \
		else                                                                                                                              \
			PushObject(CreateBoolObject(false));                                                                                          \
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

		for (size_t ip = 0; ip < frame->m_Codes.size(); ++ip)
		{
			uint8_t instruction = frame->m_Codes[ip];
			switch (instruction)
			{
			case OP_RETURN:
				return PopObject();
				break;
			case OP_NEW_FLOATING:
				PushObject(CreateRealNumObject(frame->m_RealNumNums[frame->m_Codes[++ip]]));
				break;
			case OP_NEW_INTEGER:
				PushObject(CreateIntNumObject(frame->m_IntNumNums[frame->m_Codes[++ip]]));
				break;
			case OP_NEW_STR:
				PushObject(CreateStrObject(frame->m_Strings[frame->m_Codes[++ip]]));
				break;
			case OP_NEW_TRUE:
				PushObject(CreateBoolObject(true));
				break;
			case OP_NEW_FALSE:
				PushObject(CreateBoolObject(false));
				break;
			case OP_NEW_NIL:
				PushObject(CreateNilObject());
				break;
			case OP_NEG:
			{
				Object *object = PopObject();
				if (IS_FLOATING_OBJ(object))
					PushObject(CreateRealNumObject(-TO_FLOATING_OBJ(object)->value));
				else if (IS_INTEGER_OBJ(object))
					PushObject(CreateIntNumObject(-TO_INTEGER_OBJ(object)->value));
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
				INTEGER_BINARY(%);
				break;
			case OP_BIT_AND:
				INTEGER_BINARY(&);
				break;
			case OP_BIT_OR:
				INTEGER_BINARY(|);
				break;
			case OP_BIT_XOR:
				INTEGER_BINARY(^);
				break;
			case OP_BIT_NOT:
			{
				Object *object = PopObject();
				if (IS_INTEGER_OBJ(object))
					PushObject(CreateIntNumObject(~TO_INTEGER_OBJ(object)->value));
				else
					Assert("Invalid op:'~'" + object->Stringify());
				break;
			}
			case OP_BIT_LEFT_SHIFT:
				INTEGER_BINARY(<<);
				break;
			case OP_BIT_RIGHT_SHIFT:
				INTEGER_BINARY(>>);
				break;
			case OP_GREATER:
				COMPARE_BINARY(>);
				break;
			case OP_LESS:
				COMPARE_BINARY(<);
				break;
			case OP_LESS_EQUAL:
				COMPARE_BINARY(<=);
				break;
			case OP_EQUAL:
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
				Object *value = PopObject();
				m_Context->DefineVariableByName(frame->m_Strings[frame->m_Codes[++ip]], value);
				break;
			}
			case OP_SET_VAR:
			{
				std::string name = frame->m_Strings[frame->m_Codes[++ip]];

				Object *value = PopObject();
				Object *variable = m_Context->GetVariableByName(name);

				if (IS_REF_OBJ(variable))
				{
					m_Context->AssignVariableByAddress(TO_REF_OBJ(variable)->address, value);
					TO_REF_OBJ(variable)->address = PointerAddressToString(value); //update ref address
				}
				else
					m_Context->AssignVariableByName(name, value);
				break;
			}
			case OP_GET_VAR:
			{
				std::string name = frame->m_Strings[frame->m_Codes[++ip]];

				Object *varObject = m_Context->GetVariableByName(name);
				if (IS_REF_OBJ(varObject))
					varObject = m_Context->GetVariableByAddress(TO_REF_OBJ(varObject)->address);
				PushObject(varObject);
				break;
			}
			case OP_NEW_ARRAY:
			{
				std::vector<Object *> elements;
				int64_t arraySize = (int64_t)frame->m_IntNumNums[frame->m_Codes[++ip]];
				for (int64_t i = 0; i < arraySize; ++i)
					elements.insert(elements.begin(), PopObject());
				PushObject(CreateArrayObject(elements));
				break;
			}
			case OP_NEW_TABLE:
			{
				std::unordered_map<Object *, Object *> elements;
				int64_t tableSize = (int64_t)frame->m_IntNumNums[frame->m_Codes[++ip]];
				for (int64_t i = 0; i < tableSize; ++i)
				{
					Object *key = PopObject();
					Object *value = PopObject();
					elements[key] = value;
				}
				PushObject(CreateTableObject(elements));
				break;
			}
			case OP_NEW_CLASS:
			{
				std::string name = frame->m_Strings[frame->m_Codes[++ip]];
				Object *obj;
				if (frame->HasClassFrame(name))
					obj = Execute(frame->GetClassFrame(name));
				else
					Assert("No class declaration:" + name);

				PushObject(CreateClassObject(name, m_Context->GetValues()));
				Context *up = m_Context->GetUpContext();
				if (m_Context != nullptr)
					delete m_Context;
				m_Context = up;
				break;
			}
			case OP_GET_INDEX_VAR:
			{
				Object *index = PopObject();
				Object *object = PopObject();
				if (IS_ARRAY_OBJ(object))
				{
					ArrayObject *arrayObject = TO_ARRAY_OBJ(object);
					if (!IS_INTEGER_OBJ(index))
						Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index->Stringify());

					int64_t iIndex = (int64_t)TO_INTEGER_OBJ(index)->value;

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
						PushObject(CreateNilObject());
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
					if (!IS_INTEGER_OBJ(index))
						Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index->Stringify());

					int64_t iIndex = TO_INTEGER_OBJ(index)->value;

					if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
						Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

					arrayObject->elements[iIndex] = assigner;
				}
				else if (IS_TABLE_OBJ(object))
				{
					TableObject *tableObject = TO_TABLE_OBJ(object);
					tableObject->elements[index] = assigner;
				}
				else
					Assert("Invalid index op.The indexed object isn't a array object or a table object:" + object->Stringify());
				break;
			}
			case OP_GET_CLASS_VAR:
			{
				std::string memberName = frame->m_Strings[frame->m_Codes[++ip]];
				Object *stackTop = PopObject();
				if (!IS_CLASS_OBJ(stackTop))
					Assert("Not a class object of the callee of:" + memberName);
				ClassObject *classObj = TO_CLASS_OBJ(stackTop);
				PushObject(classObj->GetPublicMember(memberName));
				break;
			}
			case OP_SET_CLASS_VAR:
			{
				std::string memberName = frame->m_Strings[frame->m_Codes[++ip]];
				Object *stackTop = PopObject();
				if (!IS_CLASS_OBJ(stackTop))
					Assert("Not a class object of the callee of:" + memberName);
				ClassObject *classObj = TO_CLASS_OBJ(stackTop);

				Object *assigner = PopObject();

				classObj->AssignPublicMember(memberName, assigner);
				break;
			}
			case OP_GET_CLASS_FUNCTION:
			{
				std::string memberName = frame->m_Strings[frame->m_Codes[++ip]];
				Object* stackTop = PopObject();
				if (!IS_CLASS_OBJ(stackTop))
					Assert("Not a class object of the callee of:" + memberName);
				ClassObject* classObj = TO_CLASS_OBJ(stackTop);
				std::string classType = classObj->name;

				Frame* classFrame = nullptr;
				if (frame->HasClassFrame(classType)) //function:function add(){return 10;}
					classFrame = frame->GetClassFrame(classType);
				else
					Assert("No class declaration:" + classType);

				if(classFrame->HasFunctionFrame(memberName))
						PushFrame(classFrame->GetFunctionFrame(memberName));
				else if (classObj->GetPublicMember(memberName) != nullptr)//lambda:let add=function(){return 10;}
				{
					Object* lambdaObject = classObj->GetPublicMember(memberName);
					if (!IS_FUNCTION_OBJ(lambdaObject))
						Assert("No lambda object:"+memberName+" in class:" + classType);
					PushFrame(classFrame->GetLambdaFrame(TO_FUNCTION_OBJ(lambdaObject)->frameIndex));
				}
				else
					Assert("No function in class:" + memberName);

				break;
			}
			case OP_ENTER_SCOPE:
			{
				m_Context = new Context(m_Context);
				break;
			}
			case OP_EXIT_SCOPE:
			{
				Context *tmp = m_Context->GetUpContext();
				delete m_Context;
				m_Context = tmp;
				break;
			}
			case OP_JUMP_IF_FALSE:
			{
				bool isJump = !TO_BOOL_OBJ(PopObject())->value;
				uint64_t address = (uint64_t)(frame->m_IntNumNums[frame->m_Codes[++ip]]);

				if (isJump)
					ip = address;
				break;
			}
			case OP_JUMP:
			{
				uint64_t address = (uint64_t)(frame->m_IntNumNums[frame->m_Codes[++ip]]);
				ip = address;
				break;
			}
			case OP_GET_FUNCTION:
			{
				std::string fnName = frame->m_Strings[frame->m_Codes[++ip]];
				if (frame->HasFunctionFrame(fnName)) //function:function add(){return 10;}
					PushFrame(frame->GetFunctionFrame(fnName));
				else if (m_Context->GetVariableByName(fnName) != nullptr)//lambda:let add=function(){return 10;}
				{
					Object *lambdaObject = m_Context->GetVariableByName(fnName);
					if (!IS_FUNCTION_OBJ(lambdaObject))
						Assert("Not a lambda object of " + fnName);
					PushFrame(frame->GetLambdaFrame(TO_FUNCTION_OBJ(lambdaObject)->frameIndex));
				}
				else if (HasNativeFunction(fnName))
					PushFrame(new NativeFunctionFrame(fnName));
				else
					Assert("No function:" + fnName);
				break;
			}
			case OP_FUNCTION_CALL:
			{
				IntNumObject *argCount = TO_INTEGER_OBJ(PopObject());

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
				PushObject(CreateFunctionObject(frame->m_IntNumNums[frame->m_Codes[++ip]]));
				break;
			case OP_REF:
			{
				PushObject(CreateRefObject(PointerAddressToString(PopObject())));
				break;
			}
			default:
				break;
			}
		}

		return CreateNilObject();
	}

	void VM::ResetStatus()
	{
		sp = 0;
		fp = 0;
		firstObject = nullptr;
		curObjCount = 0;
		maxObjCount = INIT_OBJ_NUM_MAX;

		std::array<Object *, STACK_MAX>().swap(m_ObjectStack);

		if (m_Context != nullptr)
		{
			delete m_Context;
			m_Context = nullptr;
		}
		m_Context = new Context();
	}

	std::function<Object *(std::vector<Object *>)> VM::GetNativeFunction(std::string_view fnName)
	{
		for (const auto lib : m_Libraries)
			if (lib.second->HasNativeFunction(fnName))
				return lib.second->GetNativeFunction(fnName);
		return nullptr;
	}
	bool VM::HasNativeFunction(std::string_view name)
	{
		for (const auto lib : m_Libraries)
			if (lib.second->HasNativeFunction(name))
				return true;
		return false;
	}

	void VM::PushObject(Object *object)
	{
		m_ObjectStack[sp++] = object;
	}
	Object *VM::PopObject()
	{
		return m_ObjectStack[--sp];
	}

	void VM::PushFrame(Frame *frame)
	{
		m_FrameStack[fp++] = frame;
	}
	Frame *VM::PopFrame()
	{
		return m_FrameStack[--fp];
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
			m_ObjectStack[i]->Mark();

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