#include "Chunk.h"
#include <iomanip>
#include <sstream>
#include "Utils.h"
#include "Object.h"
namespace lws
{
	Chunk::Chunk()
	{
	}

	Chunk::Chunk(const OpCodes &opcodes, const std::vector<Value> &constants)
		: opCodes(opcodes), constants(constants)
	{
	}
	Chunk::~Chunk()
	{
	}

	std::wstring Chunk::Stringify() const
	{
		std::wstring result;
		result += OpCodeStringify(opCodes);
		for (const auto &c : constants)
		{
			if (IS_FUNCTION_VALUE(c))
				result += TO_FUNCTION_VALUE(c)->Stringify();
		}
		return result;
	}
	std::vector<uint8_t> Chunk::Serialization() const
	{
		return {};
	}

	std::wstring Chunk::OpCodeStringify(const OpCodes &opcodes) const
	{
		std::wstringstream cout;
		for (int32_t i = 0; i < opcodes.size(); ++i)
		{
			switch (opcodes[i])
			{
			case OP_CONSTANT:
			{
				auto pos = opcodes[i + 1];
				std::wstring constantStr;
				if (IS_FUNCTION_VALUE(constants[pos]))
					constantStr = (L"<fn " + TO_FUNCTION_VALUE(constants[pos])->name + L":0x" + PointerAddressToString((void *)TO_FUNCTION_VALUE(constants[pos])) + L">");
				else
					constantStr = constants[pos].Stringify();
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_CONSTANT    " << pos << L"    '" << constantStr << L"'" << std::endl;
				i++;
				break;
			}
			case OP_NULL:
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_NULL" << std::endl;
				break;
			case OP_ADD:
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_ADD" << std::endl;
				break;
			case OP_SUB:
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_SUB" << std::endl;
				break;
			case OP_MUL:
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_MUL" << std::endl;
				break;
			case OP_DIV:
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_DIV" << std::endl;
				break;
			case OP_LESS:
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_LESS" << std::endl;
				break;
			case OP_GREATER:
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_GREATER" << std::endl;
				break;
			case OP_NOT:
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_NOT" << std::endl;
				break;
			case OP_MINUS:
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_MINUS" << std::endl;
				break;
			case OP_EQUAL:
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_EQUAL" << std::endl;
				break;
			case OP_RETURN:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_RETURN    " << pos << std::endl;
				i++;
				break;
			}
			case OP_FACTORIAL:
				cout << std::setfill(L'0') << std::setw(8) << i << L"    "
					 << L"OP_FACTORIAL" << std::endl;
				break;
			case OP_ARRAY:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_ARRAY    " << pos << std::endl;
				i++;
				break;
			}
			case OP_TABLE:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_TABLE    " << pos << std::endl;
				i++;
				break;
			}
			case OP_GET_INDEX:
			{
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_GET_INDEX" << std::endl;
				break;
			}
			case OP_SET_INDEX:
			{
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_SET_INDEX" << std::endl;
				break;
			}
			case OP_JUMP_IF_FALSE:
			{
				uint16_t addressOffset = int16_t(opcodes[i + 1] << 8 | opcodes[i + 2]);
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_JUMP_IF_FALSE    " << i << "->" << i + addressOffset + 3 << std::endl;
				i += 2;
				break;
			}
			case OP_JUMP:
			{
				uint16_t addressOffset = opcodes[i + 1] << 8 | opcodes[i + 2];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_JUMP    " << i << "->" << i + addressOffset + 3 << std::endl;
				i += 2;
				break;
			}
			case OP_LOOP:
			{
				uint16_t addressOffset = opcodes[i + 1] << 8 | opcodes[i + 2];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_LOOP    " << i << "->" << i - addressOffset + 3 << std::endl;
				i += 2;
				break;
			}
			case OP_POP:
			{
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_POP" << std::endl;
				break;
			}
			case OP_SET_GLOBAL:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_SET_GLOBAL    " << pos << std::endl;
				i++;
				break;
			}
			case OP_GET_GLOBAL:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_GET_GLOBAL    " << pos << std::endl;
				i++;
				break;
			}
			case OP_SET_LOCAL:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_SET_LOCAL    " << pos << std::endl;
				i++;
				break;
			}
			case OP_GET_LOCAL:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_GET_LOCAL    " << pos << std::endl;
				i++;
				break;
			}
			case OP_SET_UPVALUE:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_SET_UPVALUE    " << pos << std::endl;
				i++;
				break;
			}
			case OP_GET_UPVALUE:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_GET_UPVALUE    " << pos << std::endl;
				i++;
				break;
			}
			case OP_CLOSE_UPVALUE:
			{
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_CLOSE_UPVALUE" << std::endl;
				break;
			}
			case OP_REF_GLOBAL:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_REF_GLOBAL    " << pos << std::endl;
				i++;
				break;
			}
			case OP_REF_LOCAL:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_REF_LOCAL    " << pos << std::endl;
				i++;
				break;
			}
			case OP_REF_INDEX_GLOBAL:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_REF_INDEX_GLOBAL    " << pos << std::endl;
				i++;
				break;
			}
			case OP_REF_INDEX_LOCAL:
			{
				auto pos = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_REF_INDEX_LOCAL    " << pos << std::endl;
				i++;
				break;
			}
			case OP_CALL:
			{
				auto argCount = opcodes[i + 1];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_CALL    " << argCount << std::endl;
				i++;
				break;
			}
			case OP_CLASS:
			{
				auto varCount = opcodes[i + 1];
				auto constCount = opcodes[i + 2];
				auto parentClassCount = opcodes[i + 3];
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_CLASS    " << varCount << L"    " << constCount << L"    " << parentClassCount << std::endl;
				i += 3;
				break;
			}
			case OP_GET_PROPERTY:
			{
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_GET_PROPERTY" << std::endl;
				break;
			}
			case OP_SET_PROPERTY:
			{
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_SET_PROPERTY" << std::endl;
				break;
			}
			case OP_GET_BASE:
			{
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_GET_BASE" << std::endl;
				break;
			}
			case OP_CLOSURE:
			{
				auto pos = opcodes[i + 1];
				std::wstring funcStr = (L"<fn " + TO_FUNCTION_VALUE(constants[pos])->name + L":0x" + PointerAddressToString((void *)TO_FUNCTION_VALUE(constants[pos])) + L">");
				cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_CLOSURE    " << pos << L"    " << funcStr << std::endl;

				i++;

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
			default:
				break;
			}
		}

		return cout.str();
	}
}