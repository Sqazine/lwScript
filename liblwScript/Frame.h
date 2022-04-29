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
		OP_NEW_CONST,
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
		OP_SAVE_TO_GLOBAL,
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
		OP_REF_INDEX,
		OP_REF_OBJECT,
		OP_SELF_INCREMENT,
		OP_SELF_DECREMENT,
		OP_FACTORIAL,
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

		std::wstring Stringify(int depth = 0);

		void Clear();

		virtual FrameType Type();

	private:
		friend class VM;
		friend class Compiler;

		void AddOpCode(uint64_t code);

		uint64_t AddRealNum(double value);
		uint64_t AddIntNum(int64_t value);
		uint64_t AddString(std::wstring_view value);

		uint64_t AddLambdaFrame(Frame *frame);
		Frame *GetLambdaFrame(uint64_t idx);
		bool HasLambdaFrame(uint64_t idx);

		void AddFunctionFrame(std::wstring_view name, Frame *frame);
		Frame *GetFunctionFrame(std::wstring_view name);
		bool HasFunctionFrame(std::wstring_view name);

		void AddFieldFrame(std::wstring_view name, Frame *frame);
		Frame *GetFieldFrame(std::wstring_view name);
		bool HasFieldFrame(std::wstring_view name);

		void AddEnumFrame(std::wstring_view name, Frame *frame);
		Frame *GetEnumFrame(std::wstring_view name);
		bool HasEnumFrame(std::wstring_view name);

		std::vector<uint64_t> mCodes;

		std::vector<double> mRealNums;
		std::vector<int64_t> mIntNums;
		std::vector<std::wstring> mStrings;

		std::vector<Frame *> mLambdaFrames;
		std::unordered_map<std::wstring, Frame *> mEnumFrames;
		std::unordered_map<std::wstring, Frame *> mFunctionFrames;
		std::unordered_map<std::wstring, Frame *> mFieldFrames;

		Frame *mParentFrame;
	};

	class NativeFunctionFrame : public Frame
	{
	public:
		NativeFunctionFrame(std::wstring_view name);
		~NativeFunctionFrame();

		const std::wstring &GetName() const;

		FrameType Type() override;

	private:
		std::wstring mNativeFuntionName;
	};
}