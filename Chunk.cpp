#include "Chunk.h"
#include <string_view>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace lwScript
{

	Chunk::Chunk()
	{
	}
	Chunk::~Chunk()
	{
	}

	void Chunk::AddOpCode(uint8_t code)
	{
		m_Codes.emplace_back(code);
	}

	uint8_t Chunk::AddObject(Object *object)
	{
		m_Objects.emplace_back(object);
		return (uint8_t)(m_Objects.size() - 1);
	}

	std::string Chunk::Stringify()
	{
#define SINGLE_INSTR_STRINGIFY(op) \
	result << std::setfill('0') << std::setw(8) << i << "     " << (#op) << "\n"

#define CONSTANT_INSTR_STRINGIFY(op) \
	result << std::setfill('0') << std::setw(8) << i << "     " << (#op) << "     " << std::to_string(m_Codes[++i]) << "    " << m_Objects[m_Codes[i]]->Stringify() << "\n"

		std::stringstream result;

		result << "OpCodes:\n";

		for (size_t i = 0; i < m_Codes.size(); ++i)
		{
			switch (m_Codes[i])
			{
			case OP_RETURN:
				SINGLE_INSTR_STRINGIFY(OP_RETURN);
				break;
			case OP_PUSH:
				CONSTANT_INSTR_STRINGIFY(OP_PUSH);
				break;
			case OP_POP:
				SINGLE_INSTR_STRINGIFY(OP_POP);
				break;
			case OP_NEG:
				SINGLE_INSTR_STRINGIFY(OP_NEG);
				break;
			case OP_REF:
				SINGLE_INSTR_STRINGIFY(OP_REF);
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
			case OP_GREATER:
				SINGLE_INSTR_STRINGIFY(OP_GREATER);
				break;
			case OP_LESS:
				SINGLE_INSTR_STRINGIFY(OP_LESS);
				break;
			case OP_GREATER_EQUAL:
				SINGLE_INSTR_STRINGIFY(OP_GREATER_EQUAL);
				break;
			case OP_LESS_EQUAL:
				SINGLE_INSTR_STRINGIFY(OP_LESS_EQUAL);
				break;
			case OP_EQUAL:
				SINGLE_INSTR_STRINGIFY(OP_EQUAL);
				break;
			case OP_LOGIC_AND:
				SINGLE_INSTR_STRINGIFY(OP_LOGIC_AND);
				break;
			case OP_LOGIC_OR:
				SINGLE_INSTR_STRINGIFY(OP_LOGIC_OR);
				break;
			case OP_GET_VAR:
				SINGLE_INSTR_STRINGIFY(OP_GET_VAR);
				break;
			case OP_DEFINE_VAR:
				SINGLE_INSTR_STRINGIFY(OP_DEFINE_VAR);
				break;
			case OP_SET_VAR:
				SINGLE_INSTR_STRINGIFY(OP_SET_VAR);
				break;
			case OP_ARRAY:
				SINGLE_INSTR_STRINGIFY(OP_ARRAY);
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
			default:
				SINGLE_INSTR_STRINGIFY(UNKNOWN);
				break;
			}
		}

		result << "\nObjects:\n";

		for (size_t i = 0; i < m_Objects.size(); ++i)
			result << std::setfill('0') << std::setw(8) << i << "     " << m_Objects[i]->Stringify() << "\n";
		return result.str();
	}
	void Chunk::Clear()
	{
		std::vector<uint8_t>().swap(m_Codes);
		std::vector<Object *>().swap(m_Objects);
	}
}