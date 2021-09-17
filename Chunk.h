#pragma once 
#include <cstdint>
#include <vector>
#include <string>
#include "Object.h"
namespace lwScript
{
	enum OpCode
	{
		OP_PUSH,
		OP_POP,
		OP_NEG,
		OP_REF,
		OP_RETURN,
		OP_ADD,
		OP_SUB,
		OP_MUL,
		OP_DIV,
		OP_GREATER,
		OP_LESS,
		OP_GREATER_EQUAL,
		OP_LESS_EQUAL,
		OP_EQUAL,
		OP_NEQUAL,
		OP_LOGIC_OR,
		OP_LOGIC_AND,
		OP_GET_VAR,
		OP_SET_VAR,
		OP_DEFINE_VAR,
		OP_ARRAY,
		OP_GET_INDEX_VAR,
		OP_SET_INDEX_VAR,
		OP_ENTER_SCOPE,
		OP_EXIT_SCOPE,
	};

	class Chunk
	{
	public:
		Chunk();
		~Chunk();

		void AddOpCode(uint8_t code);
		uint8_t AddObject(Object* object);

		std::string Stringify();

		void Clear();
	private:
		friend class VM;

		std::vector<uint8_t> m_Codes;
		std::vector<Object*> m_Objects;
	};

} // namespace lwScript
