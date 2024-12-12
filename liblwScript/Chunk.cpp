#include "Chunk.h"
#include <iomanip>
#include <sstream>
#include "Version.h"
#include "Utils.h"
#include "Object.h"
#include "Logger.h"
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

	std::vector<uint8_t> Chunk::Serialize() const
	{
		std::vector<uint8_t> result;
		result.emplace_back(uint8_t((LWSCRIPT_BINARY_FILE_MAGIC_NUMBER & 0xFF000000) >> 24));
		result.emplace_back(uint8_t((LWSCRIPT_BINARY_FILE_MAGIC_NUMBER & 0x00FF0000) >> 16));
		result.emplace_back(uint8_t((LWSCRIPT_BINARY_FILE_MAGIC_NUMBER & 0x0000FF00) >> 8));
		result.emplace_back(uint8_t((LWSCRIPT_BINARY_FILE_MAGIC_NUMBER & 0x000000FF) >> 0));

		result.emplace_back(uint8_t((LWSCRIPT_VERSION_BINARY & 0xFF000000) >> 24));
		result.emplace_back(uint8_t((LWSCRIPT_VERSION_BINARY & 0x00FF0000) >> 16));
		result.emplace_back(uint8_t((LWSCRIPT_VERSION_BINARY & 0x0000FF00) >> 8));
		result.emplace_back(uint8_t((LWSCRIPT_VERSION_BINARY & 0x000000FF) >> 0));

		auto opCodeCount = opCodes.size();
		result.emplace_back(uint8_t((opCodeCount & 0xFF000000) >> 24));
		result.emplace_back(uint8_t((opCodeCount & 0x00FF0000) >> 16));
		result.emplace_back(uint8_t((opCodeCount & 0x0000FF00) >> 8));
		result.emplace_back(uint8_t((opCodeCount & 0x000000FF) >> 0));

		result.insert(result.end(), opCodes.begin(), opCodes.end());

		auto constantsCount = constants.size();
		result.emplace_back((constantsCount & 0xFF000000) >> 24);
		result.emplace_back((constantsCount & 0x00FF0000) >> 16);
		result.emplace_back((constantsCount & 0x0000FF00) >> 8);
		result.emplace_back((constantsCount & 0x000000FF) >> 0);

		for (const auto &constant : constants)
		{
			auto bytes = constant.Serialize();

			auto size = bytes.size();
			result.emplace_back((size & 0xFF000000) >> 24);
			result.emplace_back((size & 0x00FF0000) >> 16);
			result.emplace_back((size & 0x0000FF00) >> 8);
			result.emplace_back((size & 0x000000FF) >> 0);

			result.insert(result.end(), bytes.begin(), bytes.end());
		}

		return result;
	}

	void Chunk::Deserialize(const std::vector<uint8_t> &data)
	{
		auto magicNumber = ((data[0] & 0x000000FF) << 24) |
						   ((data[1] & 0x000000FF) << 16) |
						   ((data[2] & 0x000000FF) << 8) |
						   ((data[3] & 0x000000FF) << 0);
		if (magicNumber != LWSCRIPT_BINARY_FILE_MAGIC_NUMBER)
			Logger::Error(TEXT("Invalid lwscript binary file,cannot deserialize from this file"));

		auto versionNumber = ((data[4] & 0x000000FF) << 24) |
							 ((data[5] & 0x000000FF) << 16) |
							 ((data[6] & 0x000000FF) << 8) |
							 ((data[7] & 0x000000FF) << 0);

		if (versionNumber != LWSCRIPT_VERSION_BINARY)
			Logger::Error(TEXT("Invalid lwscript binary file version of {},current version is {}"), versionNumber, LWSCRIPT_VERSION_BINARY);

		auto opCodesCount = ((data[8] & 0x000000FF) << 24) |
							((data[9] & 0x000000FF) << 16) |
							((data[10] & 0x000000FF) << 8) |
							((data[11] & 0x000000FF) << 0);

		opCodes.insert(opCodes.end(), data.begin() + 12, data.begin() + 12 + opCodesCount);

		auto idx = 12 + opCodesCount;

		auto constantsCount = ((data[idx + 0] & 0x000000FF) << 24) |
							  ((data[idx + 1] & 0x000000FF) << 16) |
							  ((data[idx + 2] & 0x000000FF) << 8) |
							  ((data[idx + 3] & 0x000000FF) << 0);
		idx += 4;
		for (int32_t i = 0; i < constantsCount; ++i)
		{
			auto constantSize = ((data[idx + 0] & 0x000000FF) << 24) |
								((data[idx + 1] & 0x000000FF) << 16) |
								((data[idx + 2] & 0x000000FF) << 8) |
								((data[idx + 3] & 0x000000FF) << 0);
			idx += 4;

			std::vector<uint8_t> constantBytes(data.begin() + idx, data.begin() + idx + constantSize);

			Value v;
			v.Deserialize(constantBytes);

			constants.emplace_back(v);
		}
	}

	STD_STRING Chunk::OpCodeToString(const OpCodes &opcodes) const
	{
#define CASE(opCode)                                                                                                 \
	case opCode:                                                                                                     \
	{                                                                                                                \
		auto tok = opCodeRelatedTokens[opcodes[i + 1]];                                                              \
		auto tokStr = tok->ToString();                                                                               \
		STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));                                             \
		tokStr += tokGap;                                                                                            \
		cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\t") << TEXT(#opCode) << std::endl; \
		i += 1;                                                                                                      \
		break;                                                                                                       \
	}

#define CASE_JUMP(opCode, op)                                                                                                                                             \
	case opCode:                                                                                                                                                          \
	{                                                                                                                                                                     \
		auto tok = opCodeRelatedTokens[opcodes[i + 1]];                                                                                                                   \
		uint16_t addressOffset = opcodes[i + 2] << 8 | opcodes[i + 3];                                                                                                    \
		auto tokStr = tok->ToString();                                                                                                                                    \
		STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));                                                                                                  \
		tokStr += tokGap;                                                                                                                                                 \
		cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\t") << TEXT(#opCode) << TEXT("\t") << i << "->" << i op addressOffset + 3 << std::endl; \
		i += 3;                                                                                                                                                           \
		break;                                                                                                                                                            \
	}

#define CASE_1(opCode)                                                                                                                    \
	case opCode:                                                                                                                          \
	{                                                                                                                                     \
		auto tok = opCodeRelatedTokens[opcodes[i + 1]];                                                                                   \
		auto pos = opcodes[i + 2];                                                                                                        \
		auto tokStr = tok->ToString();                                                                                                    \
		STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));                                                                  \
		tokStr += tokGap;                                                                                                                 \
		cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\t") << TEXT(#opCode) << TEXT("\t") << pos << std::endl; \
		i += 2;                                                                                                                           \
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
				auto tok = opCodeRelatedTokens[opcodes[i + 1]];
				auto pos = opcodes[i + 2];
				STD_STRING constantStr = constants[pos].ToString();

				auto tokStr = tok->ToString();
				STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;
				cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\tOP_CONSTANT\t") << pos << TEXT("\t'") << constantStr << TEXT("'") << std::endl;
				i += 2;
				break;
			}
			case OP_CLASS:
			{
				auto tok = opCodeRelatedTokens[opcodes[i + 1]];
				auto ctorCount = opcodes[i + 2];
				auto varCount = opcodes[i + 3];
				auto constCount = opcodes[i + 4];
				auto parentClassCount = opcodes[i + 5];
				auto tokStr = tok->ToString();
				STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;
				cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\tOP_CLASS\t") << ctorCount << TEXT("\t") << varCount << TEXT("\t") << constCount << TEXT("\t") << parentClassCount << std::endl;
				i += 5;
				break;
			}
			case OP_CLOSURE:
			{
				auto tok = opCodeRelatedTokens[opcodes[i + 1]];
				auto pos = opcodes[i + 2];
				STD_STRING funcStr = (TEXT("<fn ") + TO_FUNCTION_VALUE(constants[pos])->name + TEXT(":0x") + PointerAddressToString((void *)TO_FUNCTION_VALUE(constants[pos])) + TEXT(">"));

				auto tokStr = tok->ToString();
				STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;

				cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\tOP_CLOSURE\t") << pos << TEXT("\t") << funcStr << std::endl;

				auto upvalueCount = TO_FUNCTION_VALUE(constants[pos])->upValueCount;
				if (upvalueCount > 0)
				{
					cout << TEXT("        upvalues:") << std::endl;
					for (auto j = 0; j < upvalueCount; ++j)
					{
						cout << TEXT("                 location  ") << opcodes[++i];
						cout << TEXT(" | ");
						cout << TEXT("depth  ") << opcodes[++i] << std::endl;
					}
				}
				i += 2;
				break;
			}
			case OP_MODULE:
			{
				auto tok = opCodeRelatedTokens[opcodes[i + 1]];
				auto varCount = opcodes[i + 2];
				auto constCount = opcodes[i + 3];
				auto tokStr = tok->ToString();
				STD_STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;
				cout << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\tOP_MODULE\t") << varCount << TEXT("\t") << constCount << std::endl;
				i += 3;
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