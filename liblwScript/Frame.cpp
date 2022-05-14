#include "Frame.h"
#include "Utils.h"

namespace lws
{

	Frame::Frame()
		: mParentFrame(nullptr)
	{
	}
	Frame::Frame(Frame *parentFrame)
		: mParentFrame(parentFrame)
	{
	}
	Frame::~Frame()
	{
		Clear();
	}
	void Frame::AddOpCode(uint64_t code)
	{
		mCodes.emplace_back(code);
	}

	uint64_t Frame::AddRealNum(double value)
	{
		mRealNums.emplace_back(value);
		return mRealNums.size() - 1;
	}

	uint64_t Frame::AddIntNum(int64_t value)
	{
		mIntNums.emplace_back(value);
		return mIntNums.size() - 1;
	}

	uint64_t Frame::AddString(std::wstring_view value)
	{
		mStrings.emplace_back(value);
		return mStrings.size() - 1;
	}

	uint64_t Frame::AddLambdaFrame(Frame *frame)
	{
		Frame *rootFrame = this;
		//lambda frame save to rootframe
		if (rootFrame->mParentFrame)
		{
			while (rootFrame->mParentFrame)
				rootFrame = rootFrame->mParentFrame;
		}
		rootFrame->mLambdaFrames.emplace_back(frame);
		return rootFrame->mLambdaFrames.size() - 1;
	}

	Frame *Frame::GetLambdaFrame(uint64_t idx)
	{
		if (mParentFrame)
		{
			Frame *rootFrame = this;
			while (rootFrame->mParentFrame)
				rootFrame = rootFrame->mParentFrame;
			return rootFrame->GetLambdaFrame(idx);
		}
		else if (idx >= 0 || idx < mLambdaFrames.size())
			return mLambdaFrames[idx];
		else
			return nullptr;
	}

	bool Frame::HasLambdaFrame(uint64_t idx)
	{
		if (mParentFrame)
		{
			Frame *rootFrame = this;
			while (rootFrame->mParentFrame)
				rootFrame = rootFrame->mParentFrame;
			return rootFrame->HasLambdaFrame(idx);
		}
		else if (idx >= 0 || idx < mLambdaFrames.size())
			return true;
		else
			return false;
	}

	void Frame::AddFunctionFrame(std::wstring_view name, Frame *frame)
	{
		auto iter = mFunctionFrames.find(name.data());
		if (iter != mFunctionFrames.end())
			Assert(L"Redifinition function:" + std::wstring(name));
		mFunctionFrames[name.data()] = frame;
	}

	Frame *Frame::GetFunctionFrame(std::wstring_view name)
	{
		auto iter = mFunctionFrames.find(name.data());
		if (iter != mFunctionFrames.end())
			return iter->second;
		else if (mParentFrame)
			return mParentFrame->GetFunctionFrame(name);
		else
			return nullptr;
	}

	bool Frame::HasFunctionFrame(std::wstring_view name)
	{
		auto iter = mFunctionFrames.find(name.data());
		if (iter != mFunctionFrames.end())
			return true;
		else if (mParentFrame)
			return mParentFrame->GetFunctionFrame(name);
		else
			return false;
	}

	void Frame::AddClassFrame(std::wstring_view name, Frame *frame)
	{
		auto iter = mClassFrames.find(name.data());

		if (iter != mClassFrames.end())
			Assert(std::wstring(L"Redefinition struct:") + name.data());

		mClassFrames[name.data()] = frame;
	}

	Frame *Frame::GetClassFrame(std::wstring_view name)
	{
		auto iter = mClassFrames.find(name.data());
		if (iter != mClassFrames.end())
			return iter->second;
		else if (mParentFrame != nullptr)
			return mParentFrame->GetClassFrame(name);
		Assert(std::wstring(L"No function:") + name.data());

		return nullptr;
	}

	bool Frame::HasClassFrame(std::wstring_view name)
	{
		auto iter = mClassFrames.find(name.data());
		if (iter != mClassFrames.end())
			return true;
		else if (mParentFrame != nullptr)
			return mParentFrame->HasClassFrame(name);
		return false;
	}

	void Frame::AddEnumFrame(std::wstring_view name, Frame *frame)
	{
		auto iter = mEnumFrames.find(name.data());
		if (iter != mEnumFrames.end())
			Assert(std::wstring(L"Redefinition struct:") + name.data());

		mEnumFrames[name.data()] = frame;
	}
	Frame *Frame::GetEnumFrame(std::wstring_view name)
	{
		auto iter = mEnumFrames.find(name.data());
		if (iter != mEnumFrames.end())
			return iter->second;
		else if (mParentFrame != nullptr)
			return mParentFrame->GetClassFrame(name);
		Assert(std::wstring(L"No function:") + name.data());

		return nullptr;
	}
	bool Frame::HasEnumFrame(std::wstring_view name)
	{
		auto iter = mEnumFrames.find(name.data());
		if (iter != mEnumFrames.end())
			return true;
		else if (mParentFrame != nullptr)
			return mParentFrame->HasClassFrame(name);
		return false;
	}

	std::wstring Frame::Stringify(int depth)
	{
		std::wstring interval;
		for (size_t i = 0; i < depth; ++i)
			interval += L"\t";

#define UNARY_INSTR_STRINGIFY(op) \
	result << interval << L"\t" << std::setfill(L'0') << std::setw(8) << i << L"     " << (#op) << L"\n"

#define BINARY_INSTR_STRINGIFY(op, vec) \
	result << interval << L"\t" << std::setfill(L'0') << std::setw(8) << i << L"     " << (#op) << L"     " << vec[mCodes[++i]] << L"\n"

		std::wstringstream result;

		for (auto [key, value] : mEnumFrames)
		{
			result << interval << "Enum frame " << key << ":\n";
			result << value->Stringify(depth + 1);
		}

		for (auto [key, value] : mClassFrames)
		{
			result << interval << "Class frame " << key << ":\n";
			result << value->Stringify(depth + 1);
		}

		for (auto [key, value] : mFunctionFrames)
		{
			result << interval << "Function frame " << key << ":\n";
			result << value->Stringify(depth + 1);
		}

		for (size_t i = 0; i < mLambdaFrames.size(); ++i)
		{
			result << interval << "Lambda frame " << i << ":\n";
			result << mLambdaFrames[i]->Stringify(depth + 1);
		}

		result << interval << "OpCodes:\n";

		for (size_t i = 0; i < mCodes.size(); ++i)
		{
			switch (mCodes[i])
			{
			case OP_RETURN:
				UNARY_INSTR_STRINGIFY(OP_RETURN);
				break;
			case OP_SAVE_TO_GLOBAL:
				BINARY_INSTR_STRINGIFY(OP_SAVE_TO_GLOBAL,mStrings);
				break;
			case OP_NEW_REAL:
				BINARY_INSTR_STRINGIFY(OP_NEW_REAL, mRealNums);
				break;
			case OP_NEW_INT:
				BINARY_INSTR_STRINGIFY(OP_NEW_INT, mIntNums);
				break;
			case OP_NEW_STR:
				BINARY_INSTR_STRINGIFY(OP_NEW_STR, mStrings);
				break;
			case OP_NEW_TRUE:
				UNARY_INSTR_STRINGIFY(OP_NEW_TRUE);
				break;
			case OP_NEW_FALSE:
				UNARY_INSTR_STRINGIFY(OP_NEW_FALSE);
				break;
			case OP_NEW_NULL:
				UNARY_INSTR_STRINGIFY(OP_NEW_NULL);
				break;
			case OP_NEG:
				UNARY_INSTR_STRINGIFY(OP_NEG);
				break;
			case OP_ADD:
				UNARY_INSTR_STRINGIFY(OP_ADD);
				break;
			case OP_SUB:
				UNARY_INSTR_STRINGIFY(OP_SUB);
				break;
			case OP_MUL:
				UNARY_INSTR_STRINGIFY(OP_MUL);
				break;
			case OP_DIV:
				UNARY_INSTR_STRINGIFY(OP_DIV);
				break;
			case OP_MOD:
				UNARY_INSTR_STRINGIFY(OP_MOD);
				break;
			case OP_BIT_AND:
				UNARY_INSTR_STRINGIFY(OP_BIT_AND);
				break;
			case OP_BIT_OR:
				UNARY_INSTR_STRINGIFY(OP_BIT_OR);
				break;
			case OP_BIT_XOR:
				UNARY_INSTR_STRINGIFY(OP_BIT_XOR);
				break;
			case OP_BIT_NOT:
				UNARY_INSTR_STRINGIFY(OP_BIT_NOT);
				break;
			case OP_BIT_LEFT_SHIFT:
				UNARY_INSTR_STRINGIFY(OP_BIT_LEFT_SHIFT);
				break;
			case OP_BIT_RIGHT_SHIFT:
				UNARY_INSTR_STRINGIFY(OP_BIT_RIGHT_SHIFT);
				break;
			case OP_GREATER:
				UNARY_INSTR_STRINGIFY(OP_GREATER);
				break;
			case OP_LESS:
				UNARY_INSTR_STRINGIFY(OP_LESS);
				break;
			case OP_EQUAL:
				UNARY_INSTR_STRINGIFY(OP_EQUAL);
				break;
			case OP_NOT:
				UNARY_INSTR_STRINGIFY(OP_NOT);
				break;
			case OP_AND:
				UNARY_INSTR_STRINGIFY(OP_AND);
				break;
			case OP_OR:
				UNARY_INSTR_STRINGIFY(OP_OR);
				break;
			case OP_NEW_CONST:
				BINARY_INSTR_STRINGIFY(OP_NEW_CONST, mStrings);
				break;
			case OP_GET_VAR:
				BINARY_INSTR_STRINGIFY(OP_GET_VAR, mStrings);
				break;
			case OP_NEW_VAR:
				BINARY_INSTR_STRINGIFY(OP_NEW_VAR, mStrings);
				break;
			case OP_SET_VAR:
				BINARY_INSTR_STRINGIFY(OP_SET_VAR, mStrings);
				break;
			case OP_NEW_ARRAY:
				BINARY_INSTR_STRINGIFY(OP_NEW_ARRAY, mIntNums);
				break;
			case OP_NEW_TABLE:
				BINARY_INSTR_STRINGIFY(OP_NEW_TABLE, mIntNums);
				break;
			case OP_NEW_LAMBDA:
				BINARY_INSTR_STRINGIFY(OP_NEW_LAMBDA, mIntNums);
				break;
			case OP_NEW_CLASS:
				BINARY_INSTR_STRINGIFY(OP_NEW_CLASS, mStrings);
				break;
			case OP_GET_INDEX_VAR:
				UNARY_INSTR_STRINGIFY(OP_GET_INDEX_VAR);
				break;
			case OP_SET_INDEX_VAR:
				UNARY_INSTR_STRINGIFY(OP_SET_INDEX_VAR);
				break;
			case OP_GET_CLASS_VAR:
				BINARY_INSTR_STRINGIFY(OP_GET_CLASS_VAR, mStrings);
				break;
			case OP_SET_CLASS_VAR:
				BINARY_INSTR_STRINGIFY(OP_SET_CLASS_VAR, mStrings);
				break;
			case OP_GET_CLASS_FUNCTION:
				BINARY_INSTR_STRINGIFY(OP_GET_CLASS_FUNCTION, mStrings);
				break;
			case OP_GET_FUNCTION:
				BINARY_INSTR_STRINGIFY(OP_GET_FUNCTION, mStrings);
				break;
			case OP_ENTER_SCOPE:
				UNARY_INSTR_STRINGIFY(OP_ENTER_SCOPE);
				break;
			case OP_EXIT_SCOPE:
				UNARY_INSTR_STRINGIFY(OP_EXIT_SCOPE);
				break;
			case OP_JUMP:
				BINARY_INSTR_STRINGIFY(OP_JUMP, mIntNums);
				break;
			case OP_JUMP_IF_FALSE:
				BINARY_INSTR_STRINGIFY(OP_JUMP_IF_FALSE, mIntNums);
				break;
			case OP_FUNCTION_CALL:
				UNARY_INSTR_STRINGIFY(OP_FUNCTION_CALL);
				break;
			case OP_CONDITION:
				UNARY_INSTR_STRINGIFY(OP_CONDITION);
				break;
			case OP_REF_VARIABLE:
				BINARY_INSTR_STRINGIFY(OP_REF_VARIABLE, mStrings);
				break;
			case OP_REF_INDEX:
				BINARY_INSTR_STRINGIFY(OP_REF_INDEX, mStrings);
				break;
			case OP_REF_OBJECT:
				UNARY_INSTR_STRINGIFY(OP_REF_OBJECT);
				break;
			case OP_SELF_INCREMENT:
				UNARY_INSTR_STRINGIFY(OP_SELF_INCREMENT);
				break;
			case OP_SELF_DECREMENT:
				UNARY_INSTR_STRINGIFY(OP_SELF_DECREMENT);
				break;
			case OP_FACTORIAL :
				UNARY_INSTR_STRINGIFY(OP_FACTORIAL);
				break;
			default:
				UNARY_INSTR_STRINGIFY(OP_UNKNOWN);
				break;
			}
		}

		return result.str();
	}
	void Frame::Clear()
	{
		std::vector<uint64_t>().swap(mCodes);
		std::vector<double>().swap(mRealNums);
		std::vector<int64_t>().swap(mIntNums);
		std::vector<std::wstring>().swap(mStrings);

		for (auto funcFrame : mLambdaFrames)
			funcFrame->Clear();

		for (auto [key, value] : mClassFrames)
			value->Clear();

		for (auto [key, value] : mFunctionFrames)
			value->Clear();

		std::vector<Frame *>().swap(mLambdaFrames);
		std::unordered_map<std::wstring, Frame *>().swap(mClassFrames);
		std::unordered_map<std::wstring, Frame *>().swap(mFunctionFrames);
		if (mParentFrame)
			mParentFrame = nullptr;
	}

	FrameType Frame::Type()
	{
		return FrameType::NORMAL;
	}

	NativeFunctionFrame::NativeFunctionFrame(std::wstring_view name)
		: mNativeFuntionName(name)
	{
	}
	NativeFunctionFrame::~NativeFunctionFrame()
	{
	}

	const std::wstring &NativeFunctionFrame::GetName() const
	{
		return mNativeFuntionName;
	}

	FrameType NativeFunctionFrame::Type()
	{
		return FrameType::NATIVE_FUNCTION;
	}
}