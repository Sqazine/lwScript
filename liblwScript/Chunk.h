#pragma once
#include <vector>
#include "Config.h"
#include "Value.h"
#include "Token.h"
namespace lwscript
{
    enum OpCode
    {
        OP_CONSTANT,
        OP_NULL,
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_EQUAL,
        OP_GREATER,
        OP_LESS,
        OP_NOT,
        OP_MINUS,
        OP_BIT_AND,
        OP_BIT_OR,
        OP_BIT_XOR,
        OP_BIT_NOT,
        OP_BIT_LEFT_SHIFT,
        OP_BIT_RIGHT_SHIFT,
        OP_RETURN,
        OP_FACTORIAL,
        OP_ARRAY,
        OP_DICT,
        OP_GET_INDEX,
        OP_SET_INDEX,
        OP_JUMP_IF_FALSE,
        OP_JUMP,
        OP_LOOP,
        OP_POP,
        OP_SET_GLOBAL,
        OP_GET_GLOBAL,
        OP_SET_LOCAL,
        OP_GET_LOCAL,
        OP_GET_UPVALUE,
        OP_SET_UPVALUE,
        OP_CLOSE_UPVALUE,
        OP_REF_GLOBAL,
        OP_REF_LOCAL,
        OP_REF_INDEX_GLOBAL,
        OP_REF_INDEX_LOCAL,
        OP_REF_UPVALUE,
        OP_REF_INDEX_UPVALUE,
        OP_CALL,
        OP_CLASS,
        OP_STRUCT,
        OP_SET_PROPERTY,
        OP_GET_PROPERTY,
        OP_GET_BASE,
        OP_CLOSURE,
        OP_APPREGATE_RESOLVE,
        OP_APPREGATE_RESOLVE_VAR_ARG,
        OP_MODULE,
        OP_RESET,
    };

    using OpCodes = std::vector<uint8_t>;

    class LWSCRIPT_API Chunk
    {
    public:
        Chunk() = default;
        Chunk(const OpCodes &opcodes, const std::vector<Value> &constants);
        ~Chunk() = default;
#ifndef NDEBUG
        STD_STRING ToString() const;
#endif
        std::vector<uint8_t> Serialization() const;

        OpCodes opCodes;
        std::vector<Value> constants;
        std::vector<const Token *> opCodeRelatedTokens;

    private:
        STD_STRING OpCodeToString(const OpCodes &opcodes) const;
        uint32_t GetBiggestTokenLength() const;
    };

    bool operator==(const Chunk &left, const Chunk &right);
    bool operator!=(const Chunk &left, const Chunk &right);
}