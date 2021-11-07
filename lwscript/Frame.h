#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <deque>
#include <unordered_map>
namespace lws
{
	enum OpCode
	{
		OP_NEW_FLOATING,
		OP_NEW_INTEGER,
		OP_NEW_STR,
		OP_NEW_TRUE,
		OP_NEW_FALSE,
		OP_NEW_NIL,
		OP_NEW_ARRAY,
		OP_NEW_TABLE,
		OP_NEW_LAMBDA,
		OP_NEW_CLASS,
		OP_GET_VAR,
		OP_SET_VAR,
		OP_DEFINE_VAR,
		OP_GET_INDEX_VAR,
		OP_SET_INDEX_VAR,
		OP_GET_CLASS_VAR,
		OP_SET_CLASS_VAR,
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
		OP_ENTER_SCOPE,
		OP_EXIT_SCOPE,
		OP_JUMP,
		OP_JUMP_IF_FALSE,
		OP_FUNCTION_CALL,
		OP_CONDITION,
		OP_REF,
	};

	class Frame
	{
	public:
		Frame();
		Frame(Frame *parentFrame);
		~Frame();

		void AddOpCode(uint64_t code);
		uint64_t GetOpCodeSize() const;

		uint64_t AddFloatingNum(double value);
		uint64_t AddIntegerNum(int64_t value);
		uint64_t AddString(std::string_view value);

		std::vector<double> &GetFloatingNums();
		std::vector<int64_t> &GetIntegerNums();

		uint64_t AddLambdaFrame(Frame *frame);
		Frame *GetLambdaFrame(uint64_t idx);
		bool HasLambdaFrame(uint64_t idx);

		void AddFunctionFrame(std::string_view name, Frame* frame);
		Frame* GetFunctionFrame(std::string_view name);
		bool HasFunctionFrame(std::string_view name);

		void AddClassFrame(std::string_view name, Frame *frame);
		Frame *GetClassFrame(std::string_view name);
		bool HasClassFrame(std::string_view name);

		std::string Stringify(int depth = 0);

		void Clear();

	private:
		friend class VM;

		std::vector<uint64_t> m_Codes;

		std::vector<double> m_FloatingNums;
		std::vector<int64_t> m_IntegerNums;
		std::vector<std::string> m_Strings;

		std::vector<Frame *> m_LambdaFrames;
		std::unordered_map<std::string, Frame *> m_FunctionFrames;
		std::unordered_map<std::string, Frame *> m_ClassFrames;

		Frame *m_ParentFrame;
	};
}