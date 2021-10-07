#include "VM.h"

VM::VM()
	: m_Environment(nullptr)
{
	ResetStatus();

	m_NativeFunctions["println"] = [this](std::vector<Object *> args) -> Object *
	{
		if (args.empty())
			return nullptr;

		if (args[0]->Type() != ObjectType::STR)
			Assert("Invalid argument:The first argument of native print function must be string type.");
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
		return nullptr;
	};

	m_NativeFunctions["sizeof"] = [this](std::vector<Object *> args) -> Object *
	{
		if (args.empty() || args.size() > 1)
			Assert("Expect a array argument.");

		if (!IS_ARRAY_OBJ(args[0]))
			Assert("Not a array object.");

		return CreateNumObject(TO_ARRAY_OBJ(args[0])->elements.size());
	};
}
VM::~VM()
{
	if (m_Environment)
	{
		delete m_Environment;
		m_Environment = nullptr;
	}
	sp = 0;
	Gc();
}

NumObject *VM::CreateNumObject(double value)
{
	if (curObjCount == maxObjCount)
		Gc();

	NumObject *object = new NumObject(value);
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

StructObject *VM::CreateStructObject(Environment *environment)
{
	if (curObjCount == maxObjCount)
		Gc();

	StructObject *object = new StructObject(environment);
	object->marked = false;

	object->next = firstObject;
	firstObject = object;

	curObjCount++;

	return object;
}

Object *VM::Execute(Frame *frame)
{
	// + - * /
#define COMMON_BINARY(op)                                                                  \
	do                                                                                     \
	{                                                                                      \
		Object *left = Pop();                                                              \
		Object *right = Pop();                                                             \
		if (IS_NUM_OBJ(right) && IS_NUM_OBJ(left))                                         \
			Push(CreateNumObject(TO_NUM_OBJ(left)->value op TO_NUM_OBJ(right)->value));    \
		else                                                                               \
			Assert("Invalid binary op:" + left->Stringify() + (#op) + right->Stringify()); \
	} while (0);

// > >= < <= == !=
#define COMPARE_BINARY(op)                                                                                                  \
	do                                                                                                                      \
	{                                                                                                                       \
		Object *left = Pop();                                                                                               \
		Object *right = Pop();                                                                                              \
		if (IS_NUM_OBJ(right) && IS_NUM_OBJ(left))                                                                          \
			Push(TO_NUM_OBJ(left)->value op TO_NUM_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));   \
		else if (IS_BOOL_OBJ(right) && IS_BOOL_OBJ(left))                                                                   \
			Push(TO_BOOL_OBJ(left)->value op TO_BOOL_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false)); \
		else if (IS_NIL_OBJ(right) && IS_NIL_OBJ(left))                                                                     \
			Push(TO_NIL_OBJ(left) op TO_NIL_OBJ(right) ? CreateBoolObject(true) : CreateBoolObject(false));                 \
		else                                                                                                                \
			Push(CreateBoolObject(false));                                                                                  \
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
		case OP_NUM:
			Push(CreateNumObject(frame->m_Numbers[frame->m_Codes[++ip]]));
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
			Object *value = Pop();
			m_Environment->DefineVariable(frame->m_Strings[frame->m_Codes[++ip]], value);
			break;
		}
		case OP_SET_VAR:
		{
			std::string name = frame->m_Strings[frame->m_Codes[++ip]];
			Object *value = Pop();
			m_Environment->AssignVariable(name, value);
			break;
		}
		case OP_GET_VAR:
		{
			std::string name = frame->m_Strings[frame->m_Codes[++ip]];
			Object *variableObject = m_Environment->GetVariable(name);

			//no variable
			if (variableObject == nullptr)
			{
				if (frame->HasStructFrame(name))
					variableObject = Execute(frame->GetStructFrame(name));
				else
					Assert("No variable:" + name);
			}

			Push(variableObject);
			break;
		}
		case OP_DEFINE_ARRAY:
		{
			std::vector<Object *> elements;
			int64_t arraySize = (int64_t)frame->m_Numbers[frame->m_Codes[++ip]];
			for (int64_t i = 0; i < arraySize; ++i)
				elements.insert(elements.begin(), Pop());
			Push(CreateArrayObject(elements));
			break;
		}
		case OP_DEFINE_STRUCT:
		{
			Environment *tmp = m_Environment;
			m_Environment = m_Environment->GetUpEnvironment();
			tmp->m_UpEnvironment = nullptr; //avoid environment conflict
			Push(CreateStructObject(tmp));
			break;
		}
		case OP_GET_INDEX_VAR:
		{
			Object *object = Pop();
			Object *index = Pop();
			if (IS_ARRAY_OBJ(object))
			{
				ArrayObject *arrayObject = TO_ARRAY_OBJ(object);
				if (!IS_NUM_OBJ(index))
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
				if (!IS_NUM_OBJ(index))
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
		case OP_GET_STRUCT:
		{
			std::string structName = frame->m_Strings[frame->m_Codes[++ip]];
			Object *obj = m_Environment->GetVariable(structName);
			if (!IS_STRUCT_OBJ(obj))
				Assert("Not a struct object:" + structName);

			StructObject *structObject = TO_STRUCT_OBJ(obj);
			structObject->environment->SetUpEnvironment(m_Environment);
			m_Environment = structObject->environment;
			break;
		}
		case OP_END_GET_STRUCT:
		{
			m_Environment = m_Environment->GetUpEnvironment();
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
			bool isJump = !TO_BOOL_OBJ(Pop())->value;
			uint64_t address = (uint64_t)(frame->m_Numbers[frame->m_Codes[++ip]]);

			if (isJump)
				ip = address;
			break;
		}
		case OP_JUMP:
		{
			uint64_t address = (uint64_t)(frame->m_Numbers[frame->m_Codes[++ip]]);
			ip = address;
			break;
		}
		case OP_FUNCTION_CALL:
		{
			NumObject *argCount = TO_NUM_OBJ(Pop());

			std::string fnName = frame->m_Strings[frame->m_Codes[++ip]];

			if (frame->HasFunctionFrame(fnName))
				Push(Execute(frame->GetFunctionFrame(fnName)));
			else if (HasNativeFunction(fnName))
			{
				std::vector<Object *> args;
				for (size_t i = 0; i < argCount->value; ++i)
					args.insert(args.begin(), Pop());

				Object *returnResult = GetNativeFunction(fnName)(args);
				if (returnResult != nullptr)
					Push(returnResult);
			}
			else
				Assert("No function:" + fnName);
			break;
		}
		default:
			break;
		}
	}

	return CreateNilObject();
}
void VM::AddNativeFunction(std::string_view name, std::function<Object *(std::vector<Object *> args)> fn)
{
	auto iter = m_NativeFunctions.find(name.data());
	if (iter != m_NativeFunctions.end())
		Assert(std::string("Already exists native function:") + name.data());
	m_NativeFunctions[name.data()] = fn;
}
std::function<Object *(std::vector<Object *> args)> VM::GetNativeFunction(std::string_view fnName)
{
	auto iter = m_NativeFunctions.find(fnName.data());
	if (iter != m_NativeFunctions.end())
		return iter->second;
	Assert(std::string("No native function:") + fnName.data());
}
bool VM::HasNativeFunction(std::string_view name)
{
	auto iter = m_NativeFunctions.find(name.data());
	if (iter != m_NativeFunctions.end())
		return true;
	return false;
}
void VM::ResetStatus()
{
	sp = 0;
	firstObject = nullptr;
	curObjCount = 0;
	maxObjCount = INIT_OBJ_NUM_MAX;

	std::array<Object *, STACK_MAX>().swap(m_Stack);

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