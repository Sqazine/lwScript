#pragma once
#include <vector>
#include "Config.h"
#include "Value.h"

namespace lws
{
    enum OpCode
    {
        OP_CONSTANT,
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
        OP_AND,
        OP_OR,
        OP_BIT_AND,
        OP_BIT_OR,
        OP_BIT_XOR,
        OP_BIT_NOT,
        OP_BIT_LEFT_SHIFT,
        OP_BIT_RIGHT_SHIFT,
        OP_RETURN,
        OP_FACTORIAL,
        OP_ARRAY,
        OP_TABLE,
        OP_INDEX,
        OP_JUMP_IF_FALSE,
        OP_JUMP,
        OP_POP,
        OP_SET_GLOBAL,
        OP_GET_GLOBAL,
    };

    using OpCodes = std::vector<uint8_t>;

    class Chunk
    {
    public:
        Chunk();
        Chunk(const OpCodes &opcodes, const std::vector<Value> &constants);
        ~Chunk();

        std::wstring Stringify() const;
        std::vector<uint8_t> Serialization() const;

        OpCodes opCodes;
        std::vector<Value> constants;

    private:
        std::wstring OpCodeStringify(const OpCodes &opcodes) const;
    };
}