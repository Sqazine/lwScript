#include "Frame.h"
#include "Utils.h"

namespace lws
{

	Frame::Frame()
		: m_ParentFrame(nullptr)
	{
	}
	Frame::Frame(Frame* parentFrame)
		: m_ParentFrame(parentFrame)
	{
	}
	Frame::~Frame()
	{
		Clear();
	}
	void Frame::AddOpCode(uint64_t code)
	{
		m_Codes.emplace_back(code);
	}

	uint64_t Frame::GetOpCodeSize() const
	{
		return m_Codes.size();
	}

	uint64_t Frame::AddRealNum(double value)
	{
		m_RealNums.emplace_back(value);
		return m_RealNums.size() - 1;
	}

	uint64_t Frame::AddIntNum(int64_t value)
	{
		m_IntNums.emplace_back(value);
		return m_IntNums.size() - 1;
	}

	uint64_t Frame::AddString(std::string_view value)
	{
		m_Strings.emplace_back(value);
		return m_Strings.size() - 1;
	}


	uint64_t Frame::AddLambdaFrame(Frame* frame)
	{
		Frame* rootFrame = this;
		//lambda frame save to rootframe
		if (rootFrame->m_ParentFrame)
		{
			while (rootFrame->m_ParentFrame)
				rootFrame = rootFrame->m_ParentFrame;
		}
		rootFrame->m_LambdaFrames.emplace_back(frame);
		return rootFrame->m_LambdaFrames.size() - 1;
	}

	Frame* Frame::GetLambdaFrame(uint64_t idx)
	{
		if (m_ParentFrame)
		{
			Frame* rootFrame = this;
			while (rootFrame->m_ParentFrame)
				rootFrame = rootFrame->m_ParentFrame;
			return rootFrame->GetLambdaFrame(idx);
		}
		else if (idx >= 0 || idx < m_LambdaFrames.size())
			return m_LambdaFrames[idx];
		else
			return nullptr;
	}

	bool Frame::HasLambdaFrame(uint64_t idx)
	{
		if (m_ParentFrame)
		{
			Frame* rootFrame = this;
			while (rootFrame->m_ParentFrame)
				rootFrame = rootFrame->m_ParentFrame;
			return rootFrame->HasLambdaFrame(idx);
		}
		else if (idx >= 0 || idx < m_LambdaFrames.size())
			return true;
		else
			return false;
	}

	void Frame::AddFunctionFrame(std::string_view name, Frame* frame)
	{
		auto iter = m_FunctionFrames.find(name.data());
		if (iter != m_FunctionFrames.end())
			Assert("Redifinition function:" + std::string(name));
		m_FunctionFrames[name.data()] = frame;
	}

	Frame* Frame::GetFunctionFrame(std::string_view name)
	{
		auto iter = m_FunctionFrames.find(name.data());
		if (iter != m_FunctionFrames.end())
			return iter->second;
		else if (m_ParentFrame)
			return m_ParentFrame->GetFunctionFrame(name);
		else
			return nullptr;
	}

	bool Frame::HasFunctionFrame(std::string_view name)
	{
		auto iter = m_FunctionFrames.find(name.data());
		if (iter != m_FunctionFrames.end())
			return true;
		else if (m_ParentFrame)
			return m_ParentFrame->GetFunctionFrame(name);
		else
			return false;
	}

	void Frame::AddFieldFrame(std::string_view name, Frame* frame)
	{
		auto iter = m_FieldFrames.find(name.data());

		if (iter != m_FieldFrames.end())
			Assert(std::string("Redefinition struct:") + name.data());

		m_FieldFrames[name.data()] = frame;
	}

	Frame* Frame::GetFieldFrame(std::string_view name)
	{
		auto iter = m_FieldFrames.find(name.data());
		if (iter != m_FieldFrames.end())
			return iter->second;
		else if (m_ParentFrame != nullptr)
			return m_ParentFrame->GetFieldFrame(name);
		Assert(std::string("No function:") + name.data());

		return nullptr;
	}

	bool Frame::HasFieldFrame(std::string_view name)
	{
		auto iter = m_FieldFrames.find(name.data());
		if (iter != m_FieldFrames.end())
			return true;
		else if (m_ParentFrame != nullptr)
			return m_ParentFrame->HasFieldFrame(name);
		return false;
	}

	std::string Frame::Stringify(int depth)
	{
		std::string interval;
		for (size_t i = 0; i < depth; ++i)
			interval += "\t";

#define SINGLE_INSTR_STRINGIFY(op) \
	result << interval << "\t" << std::setfill('0') << std::setw(8) << i << "     " << (#op) << "\n"

#define CONSTANT_INSTR_STRINGIFY(op, vec) \
	result << interval << "\t" << std::setfill('0') << std::setw(8) << i << "     " << (#op) << "     " << vec[m_Codes[++i]] << "\n"

		std::stringstream result;

		for (auto [key, value] : m_FieldFrames)
		{
			result << interval << "Frame " << key << ":\n";
			result << value->Stringify(depth + 1);
		}

		for (auto [key, value] : m_FunctionFrames)
		{
			result << interval << "Frame " << key << ":\n";
			result << value->Stringify(depth + 1);
		}

		for (size_t i = 0; i < m_LambdaFrames.size(); ++i)
		{
			result << interval << "Frame " << i << ":\n";
			result << m_LambdaFrames[i]->Stringify(depth + 1);
		}

		result << interval << "OpCodes:\n";

		for (size_t i = 0; i < m_Codes.size(); ++i)
		{
			switch (m_Codes[i])
			{
			case OP_RETURN:
				SINGLE_INSTR_STRINGIFY(OP_RETURN);
				break;
			case OP_NEW_REAL:
				CONSTANT_INSTR_STRINGIFY(OP_NEW_REAL, m_RealNums);
				break;
			case OP_NEW_INT:
				CONSTANT_INSTR_STRINGIFY(OP_NEW_INT, m_IntNums);
				break;
			case OP_NEW_STR:
				CONSTANT_INSTR_STRINGIFY(OP_NEW_STR, m_Strings);
				break;
			case OP_NEW_TRUE:
				SINGLE_INSTR_STRINGIFY(OP_NEW_TRUE);
				break;
			case OP_NEW_FALSE:
				SINGLE_INSTR_STRINGIFY(OP_NEW_FALSE);
				break;
			case OP_NEW_NULL:
				SINGLE_INSTR_STRINGIFY(OP_NEW_NULL);
				break;
			case OP_NEG:
				SINGLE_INSTR_STRINGIFY(OP_NEG);
				break;
			case OP_ADD:
				SINGLE_INSTR_STRINGIFY(OP_ADD);
				break;
			case OP_SUB:
				SINGLE_INSTR_STRINGIFY(OP_SUB);
				break;
			case OP_MUL:
				SINGLE_INSTR_STRINGIFY(OP_MUL);
				break;
			case OP_DIV:
				SINGLE_INSTR_STRINGIFY(OP_DIV);
				break;
			case OP_MOD:
				SINGLE_INSTR_STRINGIFY(OP_MOD);
				break;
			case OP_BIT_AND:
				SINGLE_INSTR_STRINGIFY(OP_BIT_AND);
				break;
			case OP_BIT_OR:
				SINGLE_INSTR_STRINGIFY(OP_BIT_OR);
				break;
			case OP_BIT_XOR:
				SINGLE_INSTR_STRINGIFY(OP_BIT_XOR);
				break;
			case OP_BIT_NOT:
				SINGLE_INSTR_STRINGIFY(OP_BIT_NOT);
				break;
			case OP_BIT_LEFT_SHIFT:
				SINGLE_INSTR_STRINGIFY(OP_BIT_LEFT_SHIFT);
				break;
			case OP_BIT_RIGHT_SHIFT:
				SINGLE_INSTR_STRINGIFY(OP_BIT_RIGHT_SHIFT);
				break;
			case OP_GREATER:
				SINGLE_INSTR_STRINGIFY(OP_GREATER);
				break;
			case OP_LESS:
				SINGLE_INSTR_STRINGIFY(OP_LESS);
				break;
			case OP_EQUAL:
				SINGLE_INSTR_STRINGIFY(OP_EQUAL);
				break;
			case OP_NOT:
				SINGLE_INSTR_STRINGIFY(OP_NOT);
				break;
			case OP_AND:
				SINGLE_INSTR_STRINGIFY(OP_AND);
				break;
			case OP_OR:
				SINGLE_INSTR_STRINGIFY(OP_OR);
				break;
			case OP_GET_VAR:
				CONSTANT_INSTR_STRINGIFY(OP_GET_VAR, m_Strings);
				break;
			case OP_NEW_VAR:
				CONSTANT_INSTR_STRINGIFY(OP_NEW_VAR, m_Strings);
				break;
			case OP_SET_VAR:
				CONSTANT_INSTR_STRINGIFY(OP_SET_VAR, m_Strings);
				break;
			case OP_NEW_ARRAY:
				CONSTANT_INSTR_STRINGIFY(OP_NEW_ARRAY, m_IntNums);
				break;
			case OP_NEW_TABLE:
				CONSTANT_INSTR_STRINGIFY(OP_NEW_TABLE, m_IntNums);
				break;
			case OP_NEW_LAMBDA:
				CONSTANT_INSTR_STRINGIFY(OP_NEW_LAMBDA, m_IntNums);
				break;
			case OP_NEW_FIELD:
				CONSTANT_INSTR_STRINGIFY(OP_NEW_FIELD, m_Strings);
				break;
			case OP_GET_INDEX_VAR:
				SINGLE_INSTR_STRINGIFY(OP_GET_INDEX_VAR);
				break;
			case OP_SET_INDEX_VAR:
				SINGLE_INSTR_STRINGIFY(OP_SET_INDEX_VAR);
				break;
			case OP_GET_FIELD_VAR:
				CONSTANT_INSTR_STRINGIFY(OP_GET_FIELD_VAR, m_Strings);
				break;
			case OP_SET_FIELD_VAR:
				CONSTANT_INSTR_STRINGIFY(OP_SET_FIELD_VAR, m_Strings);
				break;
			case OP_GET_FIELD_FUNCTION:
				CONSTANT_INSTR_STRINGIFY(OP_GET_FIELD_FUNCTION, m_Strings);
				break;
			case OP_GET_FUNCTION:
				CONSTANT_INSTR_STRINGIFY(OP_GET_FUNCTION, m_Strings);
				break;
			case OP_ENTER_SCOPE:
				SINGLE_INSTR_STRINGIFY(OP_ENTER_SCOPE);
				break;
			case OP_EXIT_SCOPE:
				SINGLE_INSTR_STRINGIFY(OP_EXIT_SCOPE);
				break;
			case OP_JUMP:
				CONSTANT_INSTR_STRINGIFY(OP_JUMP, m_IntNums);
				break;
			case OP_JUMP_IF_FALSE:
				CONSTANT_INSTR_STRINGIFY(OP_JUMP_IF_FALSE, m_IntNums);
				break;
			case OP_FUNCTION_CALL:
				SINGLE_INSTR_STRINGIFY(OP_FUNCTION_CALL);
				break;
			case OP_CONDITION:
				SINGLE_INSTR_STRINGIFY(OP_CONDITION);
				break;
			case OP_REF:
				SINGLE_INSTR_STRINGIFY(OP_REF);
				break;
			default:
				SINGLE_INSTR_STRINGIFY(OP_UNKNOWN);
				break;
			}
		}

		return result.str();
	}
	void Frame::Clear()
	{
		std::vector<uint64_t>().swap(m_Codes);
		std::vector<double>().swap(m_RealNums);
		std::vector<int64_t>().swap(m_IntNums);
		std::vector<std::string>().swap(m_Strings);

		for (auto funcFrame : m_LambdaFrames)
			funcFrame->Clear();

		for (auto [key, value] : m_FieldFrames)
			value->Clear();

		for (auto [key, value] : m_FunctionFrames)
			value->Clear();

		std::vector<Frame*>().swap(m_LambdaFrames);
		std::unordered_map<std::string, Frame*>().swap(m_FieldFrames);
		std::unordered_map<std::string, Frame*>().swap(m_FunctionFrames);
		if (m_ParentFrame)
			m_ParentFrame = nullptr;
	}

	FrameType Frame::Type()
	{
		return FrameType::NORMAL;
	}

	NativeFunctionFrame::NativeFunctionFrame(std::string_view name)
		: m_NativeFuntionName(name)
	{
	}
	NativeFunctionFrame::~NativeFunctionFrame()
	{
	}

	const std::string& NativeFunctionFrame::GetName() const
	{
		return m_NativeFuntionName;
	}

	FrameType NativeFunctionFrame::Type()
	{
		return FrameType::NATIVE_FUNCTION;
	}
}