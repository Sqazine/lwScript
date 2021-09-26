#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <deque>
#include "Object.h"
namespace lws
{
	enum OpCode
	{
		OP_NUM,
		OP_STR,
		OP_BOOL,
		OP_NIL,
		OP_FUNCTION,
		OP_NEG,
		OP_RETURN,
		OP_ADD,
		OP_SUB,
		OP_MUL,
		OP_DIV,
		OP_GT,
		OP_LE,
		OP_GTEQ,
		OP_LEEQ,
		OP_EQ,
		OP_NEQ,
		OP_OR,
		OP_AND,
		OP_GET_VAR,
		OP_SET_VAR,
		OP_DEFINE_VAR,
		OP_DEFINE_ARRAY,
		OP_GET_INDEX_VAR,
		OP_SET_INDEX_VAR,
		OP_ENTER_SCOPE,
		OP_EXIT_SCOPE,
		OP_JUMP,
		OP_JUMP_IF_FALSE,
		OP_FUNCTION_CALL,
	};

	class Frame
	{
	public:
		void AddOpCode(uint8_t code);
		size_t GetOpCodeSize() const;

		uint8_t AddNumber(double value);
		uint8_t AddString(std::string_view value);
		uint8_t AddBoolean(bool value);

		std::vector<double>& GetNumbers();

		void AddFunctionFrame(const Frame &frame);
		size_t GetFunctionFrameSize() const;

		std::string Stringify(int depth=0);

		void Clear();

	private:

		friend class VM;

		std::vector<uint8_t> m_Codes;
		
		std::vector<double> m_Numbers;
		std::vector<std::string> m_Strings;
		std::deque<bool> m_Booleans;//避免std::vector<bool>

		std::vector<Frame> m_FunctionFrames;
	};
} 
