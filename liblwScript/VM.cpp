#include "VM.h"
#include <iostream>
#include "Utils.h"
#include "Object.h"
namespace lws
{
    VM::VM()
    {
        ResetStatus();
    }
    VM::~VM()
    {
    }

    void VM::ResetStatus()
    {
        mFrameCount=0;
        mStackTop = mValueStack;
        objectChain = nullptr;
    }

    void VM::Run(FunctionObject *mainFunc)
    {
        ResetStatus();
        CallFrame *mainCallFrame = &mFrames[mFrameCount++];
        mainCallFrame->function = mainFunc;
        mainCallFrame->ip = mainFunc->chunk.opCodes.data();
        mainCallFrame->slots = mStackTop;

        Execute();
    }

    void VM::Execute()
    {
//  - * /
#define COMMON_BINARY(op)                                                                  \
    do                                                                                     \
    {                                                                                      \
        Value right = Pop();                                                               \
        Value left = Pop();                                                                \
        if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                                     \
            Push(TO_INT_VALUE(left) op TO_INT_VALUE(right));                               \
        else if (IS_REAL_VALUE(left) && IS_REAL_VALUE(right))                              \
            Push(TO_REAL_VALUE(left) op TO_REAL_VALUE(right));                             \
        else if (IS_INT_VALUE(left) && IS_REAL_VALUE(right))                               \
            Push(TO_INT_VALUE(left) op TO_REAL_VALUE(right));                              \
        else if (IS_REAL_VALUE(left) && IS_INT_VALUE(right))                               \
            Push(TO_REAL_VALUE(left) op TO_INT_VALUE(right));                              \
        else                                                                               \
            ASSERT(L"Invalid binary op:" + left.Stringify() + (L#op) + right.Stringify()); \
    } while (0);

// & | << >>
#define INTEGER_BINARY(op)                                                                 \
    do                                                                                     \
    {                                                                                      \
        Value right = Pop();                                                               \
        Value left = Pop();                                                                \
        if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                                     \
            Push(TO_INT_VALUE(left) op TO_INT_VALUE(right));                               \
        else                                                                               \
            ASSERT(L"Invalid binary op:" + left.Stringify() + (L#op) + right.Stringify()); \
    } while (0);

// > <
#define COMPARE_BINARY(op)                                                    \
    do                                                                        \
    {                                                                         \
        Value right = Pop();                                                  \
        Value left = Pop();                                                   \
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
        if (IS_BOOL_VALUE(left) && IS_BOOL_VALUE(right))                                    \
            Push(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false)); \
        else                                                                                \
            ASSERT("Invalid op:" + left.Stringify() + (L#op) + right.Stringify());          \
    } while (0);

        CallFrame *frame = &mFrames[mFrameCount - 1];

        while (1)
        {
            auto instruction = *frame->ip++;
            switch (instruction)
            {
            case OP_RETURN:
            {
                auto retCount = *frame->ip++;
                if (retCount > 0)
                {
                    std::wcout << Pop().Stringify() << std::endl;
                }
                else //TODO: for debug only
                {
                    mFrameCount--;
                    if (mFrameCount == 0)
                        return;
                }
                break;
            }
            case OP_CONSTANT:
            {
                auto pos = EncodeUint64(frame->function->chunk.opCodes, frame->ip - frame->function->chunk.opCodes.data() - 1);
                frame->ip += 8;
                Push(frame->function->chunk.constants[pos]);
                break;
            }
            case OP_SET_GLOBAL:
            {
                auto pos = *frame->ip++;
                mGlobalVariables[pos] = Peek(0);
                break;
            }
            case OP_GET_GLOBAL:
            {
                auto pos = *frame->ip++;
                Push(mGlobalVariables[pos]);
                break;
            }
            case OP_SET_LOCAL:
            {
                auto pos = *frame->ip++;
                auto value = Peek(0);
                mValueStack[pos] = value; //now assume base ptr on the stack bottom
                break;
            }
            case OP_GET_LOCAL:
            {
                auto pos = *frame->ip++;
                Push(mValueStack[pos]); //now assume base ptr on the stack bottom
                break;
            }
            case OP_ADD:
            {
                Value left = Pop();
                Value right = Pop();

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
                    ASSERT(L"Invalid binary op:" + left.Stringify() + L" + " + right.Stringify());
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
                if (!IS_BOOL_VALUE(value))
                    ASSERT(L"Invalid op:! " + value.Stringify() + L", only !(bool expr) is available.");
                Push(!TO_BOOL_VALUE(value));
                break;
            }
            case OP_EQUAL:
            {
                Value left = Pop();
                Value right = Pop();
                Push(left == right);
                break;
            }
            case OP_MINUS:
            {
                auto value = Pop();
                if (IS_INT_VALUE(value))
                    Push(-TO_INT_VALUE(value));
                else if (IS_REAL_VALUE(value))
                    Push(-TO_REAL_VALUE(value));
                else
                    ASSERT(L"Invalid op:-" + value.Stringify() + L", only -(int||real expr) is available.");
                break;
            }
            case OP_FACTORIAL:
            {
                auto value = Pop();
                if (IS_INT_VALUE(value))
                    Push(Factorial(TO_INT_VALUE(value)));
                else
                    ASSERT(L"Invalid op:" + value.Stringify() + L"!, only (int expr)! is available.");
                break;
            }
            case OP_ARRAY:
            {
                auto count = EncodeUint64(frame->function->chunk.opCodes, frame->ip - frame->function->chunk.opCodes.data() - 1);

                frame->ip += 8;

                std::vector<Value> elements;
                auto prePtr = mStackTop - count;
                for (auto i = 0; i < count; ++i)
                    elements.emplace_back(*prePtr++);
                mStackTop -= count;
                auto arrayObject = CreateObject<ArrayObject>(elements);
                Push(arrayObject);
                break;
            }
            case OP_TABLE:
            {
                auto eCount = EncodeUint64(frame->function->chunk.opCodes, frame->ip - frame->function->chunk.opCodes.data() - 1);
                frame->ip += 8;
                ValueUnorderedMap elements;
                for (int64_t i = 0; i < (int64_t)eCount; ++i)
                {
                    auto key = Pop();
                    auto value = Pop();
                    elements[key] = value;
                }
                Push(CreateObject<TableObject>(elements));
                break;
            }
            case OP_INDEX:
            {
                auto idxValue = Pop();
                auto dsValue = Pop();
                if (IS_ARRAY_VALUE(dsValue))
                {
                    auto array = TO_ARRAY_VALUE(dsValue);
                    if (!IS_INT_VALUE(idxValue))
                        ASSERT("Invalid idx for array,only integer is available.");
                    auto intIdx = TO_INT_VALUE(idxValue);
                    if (intIdx < 0 || intIdx >= (int64_t)array->elements.size())
                        ASSERT("Idx out of range.");
                    Push(array->elements[intIdx]);
                }
                else if (IS_STR_VALUE(dsValue))
                {
                    auto str = TO_STR_VALUE(dsValue);
                    if (!IS_INT_VALUE(idxValue))
                        ASSERT("Invalid idx for array,only integer is available.");
                    auto intIdx = TO_INT_VALUE(idxValue);
                    if (intIdx < 0 || intIdx >= (int64_t)str.size())
                        ASSERT("Idx out of range.");
                    Push(CreateObject<StrObject>(str.substr(intIdx, 1)));
                }
                else if (IS_TABLE_VALUE(dsValue))
                {
                    auto table = TO_TABLE_VALUE(dsValue);

                    auto iter = table->elements.find(idxValue);

                    if (iter != table->elements.end())
                        Push(iter->second);
                    else
                        ASSERT("No key in table.");
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
                auto address = EncodeUint64(frame->function->chunk.opCodes, frame->ip - frame->function->chunk.opCodes.data() - 1);
                frame->ip += 8;
                if (IsFalsey(Peek(0)))
                    frame->ip = frame->function->chunk.opCodes.data() + address + 1;
                break;
            }
            case OP_JUMP:
            {
                auto address = EncodeUint64(frame->function->chunk.opCodes, frame->ip - frame->function->chunk.opCodes.data() - 1);
                frame->ip=frame->function->chunk.opCodes.data()+address + 1;
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
}