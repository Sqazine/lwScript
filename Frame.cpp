#include "Frame.h"

void Frame::AddOpCode(uint8_t code)
{
    m_Codes.emplace_back(code);
}

size_t Frame::GetOpCodeSize() const
{
    return m_Codes.size();
}

uint8_t Frame::AddNumber(double value)
{
    m_Numbers.emplace_back(value);
    return m_Numbers.size() - 1;
}

uint8_t Frame::AddString(std::string_view value)
{
    m_Strings.emplace_back(value);
    return m_Strings.size() - 1;
}

std::vector<double> &Frame::GetNumbers()
{
    return m_Numbers;
}

void Frame::AddFunctionFrame(const Frame &frame)
{
    m_FunctionFrames.emplace_back(frame);
}

size_t Frame::GetFunctionFrameSize() const
{
    return m_FunctionFrames.size();
}

std::string Frame::Stringify(int depth)
{
    std::string interval;
    for (size_t i = 0; i < depth; ++i)
        interval += "\t";

#define SINGLE_INSTR_STRINGIFY(op) \
    result << interval << "\t" << std::setfill('0') << std::setw(8) << i << "     " << (#op) << "\n"

#define CONSTANT_INSTR_STRINGIFY(op, vec) \
    result << interval << "\t" << std::setfill('0') << std::setw(8) << i << "     " << (#op) << "     " << std::to_string(m_Codes[++i]) << "    " << vec[m_Codes[i]] << "\n"

    std::stringstream result;

    for (size_t i = 0; i < m_FunctionFrames.size(); ++i)
    {
        result << interval << "Frame " << i << ":\n";
        result << m_FunctionFrames[i].Stringify(depth + 1);
    }

    result << interval << "Frame root:\n"
           << interval << "OpCodes:\n";

    for (size_t i = 0; i < m_Codes.size(); ++i)
    {
        switch (m_Codes[i])
        {
        case OP_RETURN:
            SINGLE_INSTR_STRINGIFY(OP_RETURN);
            break;
        case OP_NUM:
            CONSTANT_INSTR_STRINGIFY(OP_NUM, m_Numbers);
            break;
        case OP_STR:
            CONSTANT_INSTR_STRINGIFY(OP_STR, m_Strings);
            break;
        case OP_TRUE:
            SINGLE_INSTR_STRINGIFY(OP_TRUE);
            break;
        case OP_FALSE:
            SINGLE_INSTR_STRINGIFY(OP_FALSE);
            break;
        case OP_NIL:
            SINGLE_INSTR_STRINGIFY(OP_NIL);
            break;
        case OP_FUNCTION:
            CONSTANT_INSTR_STRINGIFY(OP_FUNCTION, m_Numbers);
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
        case OP_GT:
            SINGLE_INSTR_STRINGIFY(OP_GT);
            break;
        case OP_LE:
            SINGLE_INSTR_STRINGIFY(OP_LE);
            break;
        case OP_GTEQ:
            SINGLE_INSTR_STRINGIFY(OP_GTEQ);
            break;
        case OP_LEEQ:
            SINGLE_INSTR_STRINGIFY(OP_LEEQ);
            break;
        case OP_EQ:
            SINGLE_INSTR_STRINGIFY(OP_EQ);
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
        case OP_DEFINE_VAR:
            CONSTANT_INSTR_STRINGIFY(OP_DEFINE_VAR, m_Strings);
            break;
        case OP_SET_VAR:
            CONSTANT_INSTR_STRINGIFY(OP_SET_VAR, m_Strings);
            break;
        case OP_DEFINE_ARRAY:
            CONSTANT_INSTR_STRINGIFY(OP_DEFINE_ARRAY,m_Numbers);
            break;
        case OP_GET_INDEX_VAR:
            SINGLE_INSTR_STRINGIFY(OP_GET_INDEX_VAR);
            break;
        case OP_SET_INDEX_VAR:
            SINGLE_INSTR_STRINGIFY(OP_SET_INDEX_VAR);
            break;
        case OP_ENTER_SCOPE:
            SINGLE_INSTR_STRINGIFY(OP_ENTER_SCOPE);
            break;
        case OP_EXIT_SCOPE:
            SINGLE_INSTR_STRINGIFY(OP_EXIT_SCOPE);
            break;
        case OP_JUMP:
            CONSTANT_INSTR_STRINGIFY(OP_JUMP, m_Numbers);
            break;
        case OP_JUMP_IF_FALSE:
            CONSTANT_INSTR_STRINGIFY(OP_JUMP_IF_FALSE, m_Numbers);
            break;
        case OP_FUNCTION_CALL:
            SINGLE_INSTR_STRINGIFY(OP_FUNCTION_CALL);
            break;
        default:
            SINGLE_INSTR_STRINGIFY(UNKNOWN);
            break;
        }
    }

    return result.str();
}
void Frame::Clear()
{
    std::vector<uint8_t>().swap(m_Codes);
    std::vector<double>().swap(m_Numbers);
    std::vector<std::string>().swap(m_Strings);
    std::vector<Frame>().swap(m_FunctionFrames);
}