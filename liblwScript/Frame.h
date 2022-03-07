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
		OP_NEW_REAL,
		OP_NEW_INT,
		OP_NEW_STR,
		OP_NEW_TRUE,
		OP_NEW_FALSE,
		OP_NEW_NULL,
		OP_NEW_ARRAY,
		OP_NEW_TABLE,
		OP_NEW_LAMBDA,
		OP_NEW_FIELD,
		OP_NEW_VAR,
		OP_GET_VAR,
		OP_SET_VAR,
		OP_GET_INDEX_VAR,
		OP_SET_INDEX_VAR,
		OP_GET_FIELD_VAR,
		OP_SET_FIELD_VAR,
		OP_GET_FIELD_FUNCTION,
		OP_GET_FUNCTION,
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
		OP_EQUAL,
		OP_NOT,
		OP_OR,
		OP_AND,
		OP_ENTER_SCOPE,
		OP_EXIT_SCOPE,
		OP_JUMP,
		OP_JUMP_IF_FALSE,
		OP_FUNCTION_CALL,
		OP_CONDITION,
		OP_REF_VARIABLE,
	};

	enum class FrameType
	{
		NORMAL,
		NATIVE_FUNCTION
	};

#define IS_NORMAL_FRAME(f) (f->Type() == FrameType::NORMAL)
#define IS_NATIVE_FUNCTION_FRAME(f) (f->Type() == FrameType::NATIVE_FUNCTION)
#define TO_NORMAL_FRAME(f) ((Frame *)f)
#define TO_NATIVE_FUNCTION_FRAME(f) ((NativeFunctionFrame *)f)

	class Frame
	{
	public:
		Frame();
		Frame(Frame *parentFrame);
		virtual ~Frame();

		void AddOpCode(uint64_t code);
		uint64_t GetOpCodeSize() const;

		uint64_t AddRealNum(double value);
		uint64_t AddIntNum(int64_t value);
		uint64_t AddString(std::string_view value);


		uint64_t AddLambdaFrame(Frame *frame);
		Frame *GetLambdaFrame(uint64_t idx);
		bool HasLambdaFrame(uint64_t idx);

		void AddFunctionFrame(std::string_view name, Frame *frame);
		Frame *GetFunctionFrame(std::string_view name);
		bool HasFunctionFrame(std::string_view name);

		void AddFieldFrame(std::string_view name, Frame *frame);
		Frame *GetFieldFrame(std::string_view name);
		bool HasFieldFrame(std::string_view name);

		std::string Stringify(int depth = 0);

		void Clear();

		virtual FrameType Type();

	private:
		friend class VM;
		friend class Compiler;

		std::vector<uint64_t> m_Codes;

		std::vector<double> m_RealNums;
		std::vector<int64_t> m_IntNums;
		std::vector<std::string> m_Strings;

		std::vector<Frame *> m_LambdaFrames;
		std::unordered_map<std::string, Frame *> m_FunctionFrames;
		std::unordered_map<std::string, Frame *> m_FieldFrames;

		Frame *m_ParentFrame;
	};

	class NativeFunctionFrame : public Frame
	{
	public:
		NativeFunctionFrame(std::string_view name);
		~NativeFunctionFrame();

		const std::string &GetName() const;

		FrameType Type() override;

	private:
		std::string m_NativeFuntionName;
	};
}