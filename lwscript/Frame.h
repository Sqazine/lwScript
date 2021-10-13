#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <deque>
#include "Object.h"

enum OpCode
{
	OP_FLOATING,
	OP_INTEGER,
	OP_STR,
	OP_TRUE,
	OP_FALSE,
	OP_NIL,
	OP_NEG,
	OP_RETURN,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_BIT_AND,
	OP_BIT_OR,
	OP_BIT_XOR,
	OP_BIT_NOT,
	OP_BIT_LEFT_SHIFT,
	OP_BIT_RIGHT_SHIFT,
	OP_GREATER,
	OP_LESS,
	OP_GREATER_EQUAL,
	OP_LESS_EQUAL,
	OP_EQUAL,
	OP_NOT_EQUAL,
	OP_NOT,
	OP_OR,
	OP_AND,
	OP_DEFINE_VAR,
	OP_GET_VAR,
	OP_SET_VAR,
	OP_DEFINE_ARRAY,
	OP_DEFINE_TABLE,
	OP_GET_INDEX_VAR,
	OP_SET_INDEX_VAR,
	OP_DEFINE_STRUCT,
	OP_ENTER_SCOPE,
	OP_EXIT_SCOPE,
	OP_JUMP,
	OP_JUMP_IF_FALSE,
	OP_FUNCTION_CALL,
	OP_GET_STRUCT,
	OP_END_GET_STRUCT,
	OP_CONDITION,
	OP_REF,
};

class Frame
{
public:
	Frame();
	Frame(Frame* parentFrame);
	~Frame();

	void AddOpCode(uint64_t code);
	uint64_t GetOpCodeSize() const;

	uint64_t AddFloatingNum(double value);
	uint64_t AddIntegerNum(int64_t value);
	uint64_t AddString(std::string_view value);

	std::vector<double> &GetFloatingNums();
	std::vector<int64_t> &GetIntegerNums();

	void AddFunctionFrame(std::string_view name, Frame* frame);
	Frame* GetFunctionFrame(std::string_view name);
	bool HasFunctionFrame(std::string_view name);

	void AddStructFrame(std::string_view name,  Frame* frame);
	Frame* GetStructFrame(std::string_view name);
	bool HasStructFrame(std::string_view name);

	std::string Stringify(int depth = 0);

	void Clear();

private:
	friend class VM;

	std::vector<uint8_t> m_Codes;

	std::vector<double> m_FloatingNums;
	std::vector<int64_t> m_IntegerNums;
	std::vector<std::string> m_Strings;

	std::unordered_map<std::string,Frame*> m_FunctionFrames;
	std::unordered_map<std::string,Frame*> m_StructFrames;

	Frame* m_ParentFrame;
};