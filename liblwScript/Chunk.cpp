#include "Chunk.h"
#include <iomanip>
#include <sstream>
#include "Utils.h"
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
        return OpCodeStringify(opCodes);
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
                auto pos = (((uint64_t)opcodes[i + 1]) << 56) |
                           (((uint64_t)opcodes[i + 2]) << 48) |
                           (((uint64_t)opcodes[i + 3]) << 40) |
                           (((uint64_t)opcodes[i + 4]) << 32) |
                           (((uint64_t)opcodes[i + 5]) << 24) |
                           (((uint64_t)opcodes[i + 6]) << 16) |
                           (((uint64_t)opcodes[i + 7]) << 8) |
                           (((uint64_t)opcodes[i + 8]) << 0);
                cout << std::setfill(L'0') << std::setw(8) << i << L"    "
                     << L"OP_CONSTANT    " << pos << L"    '" << constants[pos].Stringify() << "'" << std::endl;
                i += 8;
                break;
            }
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
                auto pos = EncodeUint64(opcodes, i);
                cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_ARRAY    " << pos << std::endl;
                i += 8;
                break;
            }
            case OP_TABLE:
            {
                auto pos = EncodeUint64(opcodes, i);
                cout << std::setfill(L'0') << std::setw(8) << i << L"    " << L"OP_TABLE    " << pos << std::endl;
                i += 8;
                break;
            }
            case OP_INDEX:
                cout << std::setfill(L'0') << std::setw(8) << i << L"    "
                     << L"OP_INDEX" << std::endl;
                break;
            default:
                break;
            }
        }

        return cout.str();
    }
}