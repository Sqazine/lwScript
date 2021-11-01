#include "VM.h"
namespace lws
{
	VM::VM()
		: m_Context(nullptr)
	{
		ResetStatus();
		m_Libraries["IO"]=new IO(this);
		m_Libraries["DataStructure"]=new DataStructure(this);
	}
	VM::~VM()
	{
		if (m_Context)
		{
			delete m_Context;
			m_Context = nullptr;
		}
		sp = 0;
		Gc();
	}

	FloatingObject *VM::CreateFloatingObject(double value)
	{
		if (curObjCount == maxObjCount)
			Gc();

		FloatingObject *object = new FloatingObject(value);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	IntegerObject *VM::CreateIntegerObject(int64_t value)
	{
		if (curObjCount == maxObjCount)
			Gc();

		IntegerObject *object = new IntegerObject(value);
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

	ClassObject *VM::CreateClassObject(Context *context)
	{
		if (curObjCount == maxObjCount)
			Gc();

		ClassObject *object = new ClassObject(context);
		object->marked = false;

		object->next = firstObject;
		firstObject = object;

		curObjCount++;

		return object;
	}

	RefObject *VM::CreateRefObject(std::string_view refName)
	{
		if (curObjCount == maxObjCount)
			Gc();

		RefObject *refObject = new RefObject(refName);
		refObject->marked = false;

		refObject->next = firstObject;
		firstObject = refObject;

		curObjCount++;

		return refObject;
	}

	Object *VM::Execute(Frame *frame)
	{
		// + - * /
#define COMMON_BINARY(op)                                                                              \
	do                                                                                                 \
	{                                                                                                  \
		Object *left = Pop();                                                                          \
		Object *right = Pop();                                                                         \
		if (IS_INTEGER_OBJ(right) && IS_INTEGER_OBJ(left))                                             \
			Push(CreateIntegerObject(TO_INTEGER_OBJ(left)->value op TO_INTEGER_OBJ(right)->value));    \
		else if (IS_INTEGER_OBJ(right) && IS_FLOATING_OBJ(left))                                       \
			Push(CreateFloatingObject(TO_FLOATING_OBJ(left)->value op TO_INTEGER_OBJ(right)->value));  \
		else if (IS_FLOATING_OBJ(right) && IS_INTEGER_OBJ(left))                                       \
			Push(CreateFloatingObject(TO_INTEGER_OBJ(left)->value op TO_FLOATING_OBJ(right)->value));  \
		else if (IS_FLOATING_OBJ(right) && IS_FLOATING_OBJ(left))                                      \
			Push(CreateFloatingObject(TO_FLOATING_OBJ(left)->value op TO_FLOATING_OBJ(right)->value)); \
		else                                                                                           \
			Assert("Invalid binary op:" + left->Stringify() + (#op) + right->Stringify());             \
	} while (0);
// & | % << >>
#define INTEGER_BINARY(op)                                                                          \
	do                                                                                              \
	{                                                                                               \
		Object *left = Pop();                                                                       \
		Object *right = Pop();                                                                      \
		if (IS_INTEGER_OBJ(right) && IS_INTEGER_OBJ(left))                                          \
			Push(CreateIntegerObject(TO_INTEGER_OBJ(left)->value op TO_INTEGER_OBJ(right)->value)); \
		else                                                                                        \
			Assert("Invalid binary op:" + left->Stringify() + (#op) + right->Stringify());          \
	} while (0);

// > >= < <= == !=
#define COMPARE_BINARY(op)                                                                                                          \
	do                                                                                                                              \
	{                                                                                                                               \
		Object *left = Pop();                                                                                                       \
		Object *right = Pop();                                                                                                      \
		if (IS_INTEGER_OBJ(right) && IS_INTEGER_OBJ(left))                                                                          \
			Push(TO_INTEGER_OBJ(left)->value op TO_INTEGER_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));   \
		else if (IS_INTEGER_OBJ(right) && IS_FLOATING_OBJ(left))                                                                    \
			Push(TO_FLOATING_OBJ(left)->value op TO_INTEGER_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));  \
		else if (IS_FLOATING_OBJ(right) && IS_INTEGER_OBJ(left))                                                                    \
			Push(TO_INTEGER_OBJ(left)->value op TO_FLOATING_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));  \
		else if (IS_FLOATING_OBJ(right) && IS_FLOATING_OBJ(left))                                                                   \
			Push(TO_FLOATING_OBJ(left)->value op TO_FLOATING_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false)); \
		else if (IS_BOOL_OBJ(right) && IS_BOOL_OBJ(left))                                                                           \
			Push(TO_BOOL_OBJ(left)->value op TO_BOOL_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));         \
		else if (IS_NIL_OBJ(right) && IS_NIL_OBJ(left))                                                                             \
			Push(TO_NIL_OBJ(left) op TO_NIL_OBJ(right) ? CreateBoolObject(true) : CreateBoolObject(false));                         \
		else                                                                                                                        \
			Push(CreateBoolObject(false));                                                                                          \
	} while (0);

#define LOGIC_BINARY(op)                                                                                                         \
	do                                                                                                                           \
	{                                                                                                                            \
		Object *left = Pop();                                                                                                    \
		Object *right = Pop();                                                                                                   \
		if (IS_BOOL_OBJ(right) && IS_BOOL_OBJ(left))                                                                             \
			Push(((BoolObject *)left)->value op((BoolObject *)right)->value ? CreateBoolObject(true) : CreateBoolObject(false)); \
		else                                                                                                                     \
			Assert("Invalid op:" + left->Stringify() + (#op) + right->Stringify());                                              \
	} while (0);

		for (size_t ip = 0; ip < frame->m_Codes.size(); ++ip)
		{
			uint8_t instruction = frame->m_Codes[ip];
			switch (instruction)
			{
			case OP_RETURN:
				return Pop();
				break;
			case OP_FLOATING:
				Push(CreateFloatingObject(frame->m_FloatingNums[frame->m_Codes[++ip]]));
				break;
			case OP_INTEGER:
				Push(CreateIntegerObject(frame->m_IntegerNums[frame->m_Codes[++ip]]));
				break;
			case OP_STR:
				Push(CreateStrObject(frame->m_Strings[frame->m_Codes[++ip]]));
				break;
			case OP_TRUE:
				Push(CreateBoolObject(true));
				break;
			case OP_FALSE:
				Push(CreateBoolObject(false));
				break;
			case OP_NIL:
				Push(CreateNilObject());
				break;
			case OP_NEG:
			{
				Object *object = Pop();
				if (IS_FLOATING_OBJ(object))
					Push(CreateFloatingObject(-TO_FLOATING_OBJ(object)->value));
				else if (IS_INTEGER_OBJ(object))
					Push(CreateIntegerObject(-TO_INTEGER_OBJ(object)->value));
				else
					Assert("Invalid op:'-'" + object->Stringify());
				break;
			}
			case OP_NOT:
			{
				Object *object = Pop();
				if (IS_BOOL_OBJ(object))
					Push(CreateBoolObject(!TO_BOOL_OBJ(object)->value));
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
				Object *object = Pop();
				if (IS_INTEGER_OBJ(object))
					Push(CreateIntegerObject(~TO_INTEGER_OBJ(object)->value));
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
			case OP_GREATER_EQUAL:
				COMPARE_BINARY(>=);
				break;
			case OP_LESS_EQUAL:
				COMPARE_BINARY(<=);
				break;
			case OP_EQUAL:
				COMPARE_BINARY(==);
				break;
			case OP_NOT_EQUAL:
				COMPARE_BINARY(!=);
				break;
			case OP_AND:
				LOGIC_BINARY(&&);
				break;
			case OP_OR:
				LOGIC_BINARY(||);
				break;
			case OP_DEFINE_VAR:
			{
				Object *value = Pop();
				m_Context->DefineVariable(frame->m_Strings[frame->m_Codes[++ip]], value);
				break;
			}
			case OP_SET_VAR:
			{
				std::string name = frame->m_Strings[frame->m_Codes[++ip]];
				Object *value = Pop();

				Object *variable = m_Context->GetVariable(name);

				if (IS_REF_OBJ(variable))
					name = TO_REF_OBJ(variable)->refObjName;

				m_Context->AssignVariable(name, value);
				break;
			}
			case OP_GET_VAR:
			{
				std::string name = frame->m_Strings[frame->m_Codes[++ip]];
				Object *variableObject = m_Context->GetVariable(name);

				//no variable
				if (variableObject == nullptr)
				{
					if (frame->HasStructFrame(name))
						variableObject = Execute(frame->GetStructFrame(name));
					else
						Assert("No variable:" + name);
				}
				else if (IS_REF_OBJ(variableObject))
				{
					std::string refName = TO_REF_OBJ(variableObject)->refObjName;
					variableObject = m_Context->GetVariable(refName);
				}

				Push(variableObject);
				break;
			}
			case OP_DEFINE_ARRAY:
			{
				std::vector<Object *> elements;
				int64_t arraySize = (int64_t)frame->m_IntegerNums[frame->m_Codes[++ip]];
				for (int64_t i = 0; i < arraySize; ++i)
					elements.insert(elements.begin(), Pop());
				Push(CreateArrayObject(elements));
				break;
			}
			case OP_DEFINE_TABLE:
			{
				std::unordered_map<Object *, Object *> elements;
				int64_t tableSize = (int64_t)frame->m_IntegerNums[frame->m_Codes[++ip]];
				for (int64_t i = 0; i < tableSize; ++i)
				{
					Object *key = Pop();
					Object *value = Pop();
					elements[key] = value;
				}
				Push(CreateTableObject(elements));
				break;
			}
			case OP_DEFINE_CLASS:
			{
				Context *tmp = m_Context;
				m_Context = m_Context->GetUpContext();
				tmp->m_UpContext = nullptr; //avoid context conflict
				Push(CreateClassObject(tmp));
				break;
			}
			case OP_GET_INDEX_VAR:
			{
				Object *object = Pop();
				Object *index = Pop();
				if (IS_ARRAY_OBJ(object))
				{
					ArrayObject *arrayObject = TO_ARRAY_OBJ(object);
					if (!IS_INTEGER_OBJ(index))
						Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index->Stringify());

					int64_t iIndex = (int64_t)TO_INTEGER_OBJ(index)->value;

					if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
						Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

					Push(arrayObject->elements[iIndex]);
				}
				else if (IS_TABLE_OBJ(object))
				{
					TableObject *tableObject = TO_TABLE_OBJ(object);

					bool hasValue = false;
					for (const auto [key, value] : tableObject->elements)
						if (key->IsEqualTo(index))
						{
							Push(value);
							hasValue = true;
							break;
						}
					if (!hasValue)
						Push(CreateNilObject());
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
			case OP_GET_CLASS:
			{
				std::string structName = frame->m_Strings[frame->m_Codes[++ip]];
				Object *obj = m_Context->GetVariable(structName);
				if (!IS_CLASS_OBJ(obj))
					Assert("Not a struct object:" + structName);

				ClassObject *structObject = TO_CLASS_OBJ(obj);
				structObject->context->SetUpContext(m_Context);
				m_Context = structObject->context;
				break;
			}
			case OP_END_GET_CLASS:
			{
				m_Context = m_Context->GetUpContext();
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
				bool isJump = !TO_BOOL_OBJ(Pop())->value;
				uint64_t address = (uint64_t)(frame->m_FloatingNums[frame->m_Codes[++ip]]);

				if (isJump)
					ip = address;
				break;
			}
			case OP_JUMP:
			{
				uint64_t address = (uint64_t)(frame->m_FloatingNums[frame->m_Codes[++ip]]);
				ip = address;
				break;
			}
			case OP_FUNCTION_CALL:
			{
				IntegerObject *argCount = TO_INTEGER_OBJ(Pop());

				std::string fnName = frame->m_Strings[frame->m_Codes[++ip]];

				if (frame->HasFunctionFrame(fnName))
					Push(Execute(frame->GetFunctionFrame(fnName)));
				else if (HasNativeFunction(fnName))
				{
					std::vector<Object *> args;
					for (int64_t i = 0; i < argCount->value; ++i)
						args.insert(args.begin(), Pop());

					Object *returnResult = GetNativeFunction(fnName)(args);
					if (returnResult != nullptr)
						Push(returnResult);
				}
				else
					Assert("No function:" + fnName);
				break;
			}
			case OP_CONDITION:
			{
				Object *condition = Pop();
				Object *trueBranch = Pop();
				Object *falseBranch = Pop();

				if (!IS_BOOL_OBJ(condition))
					Assert("Not a bool expr of condition expr's '?'.");
				if (TO_BOOL_OBJ(condition)->value)
					Push(trueBranch);
				else
					Push(falseBranch);
				break;
			}
			case OP_REF:
			{
				Push(CreateRefObject(frame->m_Strings[frame->m_Codes[++ip]]));
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
		firstObject = nullptr;
		curObjCount = 0;
		maxObjCount = INIT_OBJ_NUM_MAX;

		std::array<Object *, STACK_MAX>().swap(m_Stack);

		if (m_Context != nullptr)
		{
			delete m_Context;
			m_Context = nullptr;
		}
		m_Context = new Context();
	}

	std::function<Object *(std::vector<Object *>)> VM::GetNativeFunction(std::string_view fnName)
	{
		for(const auto lib:m_Libraries)
				if(lib.second->HasNativeFunction(fnName))
					return lib.second->GetNativeFunction(fnName);
			return nullptr;
	}
        bool VM::HasNativeFunction(std::string_view name)
		{
			for(const auto lib:m_Libraries)
				if(lib.second->HasNativeFunction(name))
					return true;
			return false;
		}

	void VM::Push(Object *object)
	{
		m_Stack[sp++] = object;
	}
	Object *VM::Pop()
	{
		return m_Stack[--sp];
	}

	void VM::Gc()
	{
		int objNum = curObjCount;

		//mark all object which in stack;
		for (size_t i = 0; i < sp; ++i)
			m_Stack[i]->Mark();

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