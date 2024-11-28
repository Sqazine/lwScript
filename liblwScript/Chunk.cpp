#include "Chunk.h"
#include <iomanip>
#include <sstream>

#include "Utils.h"
#include "Object.h"
namespace lwscript
{
	Chunk::Chunk(const OpCodes &opcodes, const std::vector<Value> &constants)
		: opCodes(opcodes), constants(constants)
	{
	}

#ifndef NDEBUG
	STD_STRING Chunk::ToString() const
	{
		STD_STRING result;
		result += OpCodeToString(opCodes);
		for (const auto &c : constants)
		{
			if (IS_FUNCTION_VALUE(c))
				result += TO_FUNCTION_VALUE(c)->ToStringWithChunk();
		}
		return result;
	}
#endif

	std::vector<uint8_t> Chunk::Serialization() const
	{
		return {};
	}

	STD_STRING Chunk::OpCodeToString(const OpCodes &opcodes) const
	{
#define CASE(opCode)                                                                                                 \
	case opCode:                                                                                                     \
	{                                                                                                                \
		auto tok = opCodeRelatedTokens[opcodes[++i]];                                                                \
		auto tokStr = tok->ToString();                                                                               \
		STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));                                             \
		tokStr += tokGap;                                                                                            \
		cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\t") << TEXT(#opCode) << std::endl; \
		break;                                                                                                       \
	}

#define CASE_JUMP(opCode, op)                                                                                                                                             \
	case opCode:                                                                                                                                                          \
	{                                                                                                                                                                     \
		auto tok = opCodeRelatedTokens[opcodes[++i]];                                                                                                                     \
		uint16_t addressOffset = opcodes[++i] << 8 | opcodes[++i];                                                                                                        \
		auto tokStr = tok->ToString();                                                                                                                                    \
		STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));                                                                                                  \
		tokStr += tokGap;                                                                                                                                                 \
		cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\t") << TEXT(#opCode) << TEXT("\t") << i << "->" << i op addressOffset + 3 << std::endl; \
		break;                                                                                                                                                            \
	}

#define CASE_1(opCode)                                                                                                                    \
	case opCode:                                                                                                                          \
	{                                                                                                                                     \
		auto tok = opCodeRelatedTokens[opcodes[++i]];                                                                                     \
		auto pos = opcodes[++i];                                                                                                          \
		auto tokStr = tok->ToString();                                                                                                    \
		STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));                                                                  \
		tokStr += tokGap;                                                                                                                 \
		cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\t") << TEXT(#opCode) << TEXT("\t") << pos << std::endl; \
		break;                                                                                                                            \
	}

		const uint32_t maxTokenShowSize = GetBiggestTokenLength() + 4; // 4 for a gap "    "
		STD_STRING_STREAM cout;
		for (int32_t i = 0; i < opcodes.size(); ++i)
		{
			switch (opcodes[i])
			{

				CASE(OP_NULL)
				CASE(OP_ADD)
				CASE(OP_SUB)
				CASE(OP_MUL)
				CASE(OP_DIV)
				CASE(OP_LESS)
				CASE(OP_GREATER)
				CASE(OP_NOT)
				CASE(OP_MINUS)
				CASE(OP_BIT_AND)
				CASE(OP_BIT_OR)
				CASE(OP_BIT_XOR)
				CASE(OP_BIT_NOT)
				CASE(OP_BIT_LEFT_SHIFT)
				CASE(OP_BIT_RIGHT_SHIFT)
				CASE(OP_EQUAL)
				CASE(OP_FACTORIAL)
				CASE(OP_CLOSE_UPVALUE)
				CASE(OP_GET_INDEX)
				CASE(OP_SET_INDEX)
				CASE(OP_POP)
				CASE(OP_GET_BASE)
				CASE(OP_SET_PROPERTY)
				CASE(OP_GET_PROPERTY)
				CASE_JUMP(OP_JUMP_IF_FALSE, +)
				CASE_JUMP(OP_JUMP, +)
				CASE_JUMP(OP_LOOP, -)
				CASE_1(OP_RETURN)
				CASE_1(OP_ARRAY)
				CASE_1(OP_DICT)
				CASE_1(OP_SET_GLOBAL)
				CASE_1(OP_GET_GLOBAL)
				CASE_1(OP_SET_LOCAL)
				CASE_1(OP_GET_LOCAL)
				CASE_1(OP_SET_UPVALUE)
				CASE_1(OP_GET_UPVALUE)
				CASE_1(OP_REF_GLOBAL)
				CASE_1(OP_REF_LOCAL)
				CASE_1(OP_REF_INDEX_GLOBAL)
				CASE_1(OP_REF_INDEX_LOCAL)
				CASE_1(OP_REF_UPVALUE)
				CASE_1(OP_REF_INDEX_UPVALUE)
				CASE_1(OP_CALL)
				CASE_1(OP_STRUCT)
				CASE_1(OP_APPREGATE_RESOLVE)
				CASE_1(OP_APPREGATE_RESOLVE_VAR_ARG)
				CASE_1(OP_RESET)
			case OP_CONSTANT:
			{
				auto tok = opCodeRelatedTokens[opcodes[++i]];
				auto pos = opcodes[++i];
				STD_STRING constantStr = constants[pos].ToString();

				auto tokStr = tok->ToString();
				STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;
				cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\tOP_CONSTANT\t") << pos << TEXT("\t'") << constantStr << TEXT("'") << std::endl;
				break;
			}
			case OP_CLASS:
			{
				auto tok = opCodeRelatedTokens[opcodes[++i]];
				auto ctorCount = opcodes[++i];
				auto varCount = opcodes[++i];
				auto constCount = opcodes[++i];
				auto parentClassCount = opcodes[++i];
				auto tokStr = tok->ToString();
				STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;
				cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\tOP_CLASS\t") << ctorCount << TEXT("\t") << varCount << TEXT("\t") << constCount << TEXT("\t") << parentClassCount << std::endl;
				break;
			}
			case OP_CLOSURE:
			{
				auto tok = opCodeRelatedTokens[opcodes[++i]];
				auto pos = opcodes[++i];
				STD_STRING funcStr = (TEXT("<fn ") + TO_FUNCTION_VALUE(constants[pos])->name + TEXT(":0x") + PointerAddressToString((void *)TO_FUNCTION_VALUE(constants[pos])) + TEXT(">"));

				auto tokStr = tok->ToString();
				STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;

				cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\tOP_CLOSURE\t") << pos << TEXT("\t") << funcStr << std::endl;

				auto upvalueCount = TO_FUNCTION_VALUE(constants[pos])->upValueCount;
				if (upvalueCount > 0)
				{

					cout << "        upvalues:" << std::endl;
					for (auto j = 0; j < upvalueCount; ++j)
					{
						cout << "                 location  " << opcodes[++i];
						cout << " | ";
						cout << "depth  " << opcodes[++i] << std::endl;
					}
				}
				break;
			}
			case OP_MODULE:
			{
				auto tok = opCodeRelatedTokens[opcodes[++i]];
				auto varCount = opcodes[++i];
				auto constCount = opcodes[++i];
				auto tokStr = tok->ToString();
				STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;
				cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\tOP_MODULE\t") << varCount << TEXT("\t") << constCount << std::endl;
				break;
			}
			default:
				break;
			}
		}

		return cout.str();
	}

	uint32_t Chunk::GetBiggestTokenLength() const
	{
		uint32_t length = 0;
		for (const auto &t : opCodeRelatedTokens)
		{
			auto l = (uint32_t)t->ToString().size();
			if (length < l)
				length = l;
		}
		return length;
	}

	bool operator==(const Chunk &left, const Chunk &right)
	{
		if (left.opCodes != right.opCodes)
			return false;
		if (left.constants.size() != right.constants.size())
			return false;
		for (int32_t i = 0; i < left.constants.size(); ++i)
			if (left.constants[i] != right.constants[i])
				return false;
		return true;
	}

	bool operator!=(const Chunk &left, const Chunk &right)
	{
		return !(left == right);
	}
}