#include "VM.h"
#include <iostream>
#include "Utils.h"
namespace lws
{
    VM::VM()
    {
    }
    VM::~VM()
    {
    }

    void VM::ResetStatus()
    {
        mStackTop = mValueStack;
    }

    void VM::Run(const Chunk &chunk)
    {
        mChunk = chunk;
        ResetStatus();
        Execute();
    }

    void VM::Execute()
    {
//  - * /
#define COMMON_BINARY(op)                                                                  \
    do                                                                                     \
    {                                                                                      \
        Value left = Pop();                                                                \
        Value right = Pop();                                                               \
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
        Value left = Pop();                                                                \
        Value right = Pop();                                                               \
        if (IS_INT_VALUE(left) && IS_INT_VALUE(right))                                     \
            Push(TO_INT_VALUE(left) op TO_INT_VALUE(right));                               \
        else                                                                               \
            ASSERT(L"Invalid binary op:" + left.Stringify() + (L#op) + right.Stringify()); \
    } while (0);

// > <
#define COMPARE_BINARY(op)                                                    \
    do                                                                        \
    {                                                                         \
        Value left = Pop();                                                   \
        Value right = Pop();                                                  \
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
        Value left = Pop();                                                                 \
        Value right = Pop();                                                                \
        if (IS_BOOL_VALUE(left) && IS_BOOL_VALUE(right))                                    \
            Push(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false)); \
        else                                                                                \
            ASSERT("Invalid op:" + left.Stringify() + (L#op) + right.Stringify());          \
    } while (0);

        for (int32_t i = 0; i < mChunk.opCodes.size(); ++i)
        {
            auto instruction = mChunk.opCodes[i];
            switch (instruction)
            {
            case OP_RETURN:
            {
                auto retCount = mChunk.opCodes[++i];
                if (retCount > 0)
                    std::wcout << Pop().Stringify() << std::endl;
                break;
            }
            case OP_CONSTANT:
            {
                auto pos = (((uint64_t)mChunk.opCodes[i + 1]) << 56) |
                           (((uint64_t)mChunk.opCodes[i + 2]) << 48) |
                           (((uint64_t)mChunk.opCodes[i + 3]) << 40) |
                           (((uint64_t)mChunk.opCodes[i + 4]) << 32) |
                           (((uint64_t)mChunk.opCodes[i + 5]) << 24) |
                           (((uint64_t)mChunk.opCodes[i + 6]) << 16) |
                           (((uint64_t)mChunk.opCodes[i + 7]) << 8) |
                           (((uint64_t)mChunk.opCodes[i + 8]) << 0);
                i += 8;
                Push(mChunk.constants[pos]);
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
            default:
                break;
            }
        }
    }

    void VM::Push(const Value &value)
    {
        *(mStackTop++) = value;
    }
    Value VM::Pop()
    {
        return *(--mStackTop);
    }
}