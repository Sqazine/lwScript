#include "VM.h"
#include <iostream>
#include "Constant.h"
#include "Object.h"
namespace lwScript
{
	 Environment::Environment()
        : m_UpEnvironment(nullptr)
    {
    }

    Environment::Environment(Environment *upEnvironment)
        : m_UpEnvironment(upEnvironment)
    {
    }

    Environment::~Environment()
    {
    }

    void Environment::DefineVariable(std::string_view name, Object *value)
    {
        auto iter = m_Values.find(name.data());
        if (iter != m_Values.end())
        {
            std::cout << "Redefined variable:" << name << " in current context." << std::endl;
            exit(1);
        }
        else
            m_Values[name.data()] = value;
    }

    void Environment::AssignVariable(std::string_view name, Object *value)
    {
        auto iter = m_Values.find(name.data());
        if (iter != m_Values.end())
            m_Values[name.data()] = value;
        else if (m_UpEnvironment != nullptr)
            m_UpEnvironment->AssignVariable(name, value);
        else
        {
            std::cout << "Undefine variable:" << name << " in current context" << std::endl;
            exit(1);
        }
    }

    Object *Environment::GetVariable(std::string_view name)
    {
        auto iter = m_Values.find(name.data());

        if (iter != m_Values.end())
            return iter->second;

        if (m_UpEnvironment != nullptr)
            return m_UpEnvironment->GetVariable(name);

        return nilObject;
    }

    Environment *Environment::GetUpEnvironment()
    {
        return m_UpEnvironment;
    }

    const std::unordered_map<std::string, Object *> &Environment::GetValues() const
    {
        return m_Values;
    }

	VM::VM()
		: m_Environment(nullptr)
	{
		ResetStatus();
	}
	VM::~VM()
	{
		if (m_Environment)
		{
			delete m_Environment;
			m_Environment = nullptr;
		}
	}
	void VM::Execute(const Frame &frame)
	{
#define TO_NUM_OBJ(obj) ((NumObject *)obj)
#define TO_STR_OBJ(obj) ((StrObject *)obj)
#define TO_NIL_OBJ(obj) ((NilObject *)obj)
#define TO_BOOL_OBJ(obj) ((BoolObject *)obj)
#define TO_ARRAY_OBJ(obj) ((ArrayObject *)obj)

// + - * /
#define COMMON_BINARY(op)                                                                                    \
	{                                                                                                           \
		Object *left = Pop();                                                                                   \
		Object *right = Pop();                                                                                  \
		if (right->Type() == ObjectType::NUM && left->Type() == ObjectType::NUM)                                          \
		{                                                                                                       \
			auto t = new NumObject();                                                                           \
			t->value = TO_NUM_OBJ(left)->value op TO_NUM_OBJ(right)->value;                                     \
			Push(t);                                                                                            \
		}                                                                                                       \
                                                                                            \
		else                                                                                                    \
		{                                                                                                       \
			std::cout << "Invalid binary op:" << left->Stringify() << (#op) << right->Stringify() << std::endl; \
			exit(1);                                                                                            \
		}                                                                                                       \
	}

// > >= < <= == !=
#define COMPARE_BINARY(op)                                                                         \
	{                                                                                                 \
		Object *left = Pop();                                                                         \
		Object *right = Pop();                                                                        \
		if (right->Type() == ObjectType::NUM && left->Type() == ObjectType::NUM)                                \
			Push(TO_NUM_OBJ(left)->value op TO_NUM_OBJ(right)->value ? trueObject : falseObject);     \
		else if (right->Type() == ObjectType::BOOL && left->Type() ==ObjectType:: BOOL)                         \
			Push(TO_BOOL_OBJ(left)->value op TO_BOOL_OBJ(right)->value ? trueObject : falseObject);   \
		else if (right->Type() ==ObjectType:: NIL && left->Type() ==ObjectType:: NIL)                         \
			Push(TO_NIL_OBJ(left) op TO_NIL_OBJ(right) ? trueObject : falseObject);                 \
		else                                                                                          \
			Push(falseObject);                                                                        \
	}

		ResetStatus();
		m_RootFrame = frame;
		for (; ip < m_RootFrame.m_Codes.size(); ++ip)
		{
			uint8_t instruction = m_RootFrame.m_Codes[ip];
			switch (instruction)
			{
			case OP_RETURN:
				std::cout << Pop()->Stringify() << std::endl;
				break;
			case OP_PUSH:
				Push(m_RootFrame.m_Objects[m_RootFrame.m_Codes[++ip]]);
				break;
			case OP_NEG:
			{
				Object *object = Pop();
				if (object->Type() == ObjectType::NUM)
				{
					auto t = TO_NUM_OBJ(object);
					t->value *= -1.0;
					Push(t);
				}
				else
				{
					std::cout << "Invalid op:'-'" << object->Stringify() << std::endl;
					exit(1);
				}
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
			case OP_LOGIC_AND:
			{
				Object *left = Pop();
				Object *right = Pop();

				if (right->Type() == ObjectType::BOOL && left->Type() == ObjectType::BOOL)
					Push(((BoolObject *)left)->value && ((BoolObject *)right)->value ? trueObject : falseObject);
				else
				{
					std::cout << "Invalid op:" << left->Stringify() << "&&" << right->Stringify() << std::endl;
					exit(1);
				}
				break;
			}
			case OP_LOGIC_OR:
			{
				Object *left = Pop();
				Object *right = Pop();

				if (right->Type() == ObjectType::BOOL && left->Type() == ObjectType::BOOL)
					Push(((BoolObject *)left)->value || ((BoolObject *)right)->value ? trueObject : falseObject);
				else
				{
					std::cout << "Invalid op:" << left->Stringify() << "||" << right->Stringify() << std::endl;
					exit(1);
				}
				break;
			}
			case OP_DEFINE_VAR:
			{
				Object *variableName = Pop();
				Object *value = Pop();
				m_Environment->DefineVariable(((StrObject *)variableName)->value, value);
				break;
			}
			case OP_SET_VAR:
			{
				StrObject *variableName = TO_STR_OBJ(Pop());
				Object *value = Pop();
				m_Environment->AssignVariable(variableName->value, value);
				break;
			}
			case OP_GET_VAR:
			{
				Object *variableName = Pop();
				Push(m_Environment->GetVariable(((StrObject *)variableName)->value));
				break;
			}
			case OP_ARRAY:
			{
				ArrayObject *arrayObject = new ArrayObject();
				NumObject *arraySize = TO_NUM_OBJ(Pop());
				for (int64_t i = 0; i <(int64_t) arraySize->value; ++i)
					arrayObject->elements.insert(arrayObject->elements.begin(), Pop());
				Push(arrayObject);
				break;
			}
			case OP_GET_INDEX_VAR:
			{
				Object *object = Pop();
				Object *index = Pop();
				if (object->Type() == ObjectType::ARRAY)
				{
					ArrayObject *arrayObject = TO_ARRAY_OBJ(object);
					if (index->Type() != ObjectType::NUM)
					{
						std::cout << "Invalid index op.The index type of the array object must ba a int num type,but got:" << index->Stringify() << std::endl;
						exit(1);
					}


					int64_t iIndex =(int64_t) TO_NUM_OBJ(index)->value;

					if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
					{
						std::cout << "Index out of array range,array size:" << arrayObject->elements.size() << ",index:" << iIndex << std::endl;
						exit(1);
					}

					Push(arrayObject->elements[iIndex]);
				}
				else
				{
					std::cout << "Invalid index op.The indexed object isn't a array object or a table object:" << object->Stringify() << std::endl;
					exit(1);
				}
				break;
			}
			case OP_SET_INDEX_VAR:
			{
				Object *object = Pop();
				Object *index = Pop();
				Object *assigner = Pop();

				if (object->Type() ==ObjectType::ARRAY)
				{
					ArrayObject *arrayObject = TO_ARRAY_OBJ(object);
					if (index->Type() != ObjectType::NUM)
					{
						std::cout << "Invalid index op.The index type of the array object must ba a int num type,but got:" << index->Stringify() << std::endl;
						exit(1);
					}

					int64_t iIndex =(int64_t)TO_NUM_OBJ(index)->value;

					if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
					{
						std::cout << "Index out of array range,array size:" << arrayObject->elements.size() << ",index:" << iIndex << std::endl;
						exit(1);
					}

					arrayObject->elements[iIndex] = assigner;
				}
				else
				{
					std::cout << "Invalid index op.The indexed object isn't a array object or a table object:" << object->Stringify() << std::endl;
					exit(1);
				}
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
			case OP_STRUCT:
			{
				StructObject* structObject=new StructObject();
				NumObject* memberCount=TO_NUM_OBJ(Pop());
				for(size_t i=0;i<memberCount->value;++i)
				{
					StrObject* variableName=TO_STR_OBJ(Pop());
					structObject->variables[variableName->value]=Pop();
				}
				Push(structObject);
				break;
			}
			default:
				break;
			}
		}
	}
	void VM::ResetStatus()
	{
		ip = sp = 0;
		std::array<Object *, 64>().swap(m_Stack);

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