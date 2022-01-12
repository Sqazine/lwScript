#include "Lexer.h"
namespace lws
{
    static std::unordered_map<std::string, TokenType> keywords =
        {
            {"let", TokenType::LET},
            {"if", TokenType::IF},
            {"else", TokenType::ELSE},
            {"true", TokenType::TRUE},
            {"false", TokenType::FALSE},
            {"nil", TokenType::NIL},
            {"while", TokenType::WHILE},
            {"for",TokenType::FOR},
            {"fn", TokenType::FUNCTION},
            {"field", TokenType::FIELD},
            {"return", TokenType::RETURN},
    };

    Lexer::Lexer()
    {
        ResetStatus();
    }
    Lexer::~Lexer()
    {
    }

    const std::vector<Token> &Lexer::GenerateTokens(std::string_view src)
    {
        ResetStatus();
        m_Source = src;
        while (!IsAtEnd())
        {
            m_StartPos = m_CurPos;
            GenerateToken();
        }

        AddToken(TokenType::END, "EOF");

        return m_Tokens;
    }
    void Lexer::GenerateToken()
    {
        char c = GetCurCharAndStepOnce();

        switch (c)
        {
        case '(':
            AddToken(TokenType::LPAREN);
            break;
        case ')':
            AddToken(TokenType::RPAREN);
            break;
        case '[':
            AddToken(TokenType::LBRACKET);
            break;
        case ']':
            AddToken(TokenType::RBRACKET);
            break;
        case '{':
            AddToken(TokenType::LBRACE);
            break;
        case '}':
            AddToken(TokenType::RBRACE);
            break;
        case '.':
            AddToken(TokenType::DOT);
            break;
        case ',':
            AddToken(TokenType::COMMA);
            break;
        case ':':
            AddToken(TokenType::COLON);
            break;
        case ';':
            AddToken(TokenType::SEMICOLON);
            break;
        case '\"':
            String();
            break;
        case ' ':
        case '\t':
        case '\r':
            break;
        case '\n':
            m_Line++;
            break;
        case '+':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::PLUS_EQUAL);
            else
                AddToken(TokenType::PLUS);
            break;
        case '-':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::MINUS_EQUAL);
            else
                AddToken(TokenType::MINUS);
            break;
        case '*':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::ASTERISK_EQUAL);
            else
                AddToken(TokenType::ASTERISK);
            break;
        case '/':
            if (IsMatchCurCharAndStepOnce('/'))
            {
                while (!IsMatchCurChar('\n') && !IsAtEnd())
                    GetCurCharAndStepOnce();
                m_Line++;
                break;
            }
            else if (IsMatchCurCharAndStepOnce('*'))
            {
                while (!IsMatchCurChar('*') && !IsMatchNextChar('/') && !IsAtEnd())
                {
                    if (IsMatchCurChar('\n'))
                        m_Line++;
                    GetCurCharAndStepOnce();
                }
                GetCurCharAndStepOnce(); //eat '*'
                GetCurCharAndStepOnce(); // eat '/'
            }
            else if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::SLASH_EQUAL);
            else
                AddToken(TokenType::SLASH);
            break;
        case '%':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::PERCENT_EQUAL);
            AddToken(TokenType::PERCENT);
            break;
        case '~':
            AddToken(TokenType::TILDE);
            break;
        case '!':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::BANG_EQUAL);
            else
                AddToken(TokenType::BANG);
            break;
        case '&':
            if (IsMatchCurCharAndStepOnce('&'))
                AddToken(TokenType::AMPERSAND_AMPERSAND);
            else if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::AMPERSAND_EQUAL);
            else
                AddToken(TokenType::AMPERSAND);
            break;
        case '|':
            if (IsMatchCurCharAndStepOnce('|'))
                AddToken(TokenType::VBAR_VBAR);
            else if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::VBAR_EQUAL);
            else
                AddToken(TokenType::VBAR);
            break;
        case '^':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::CARET_EQUAL);
            else
                AddToken(TokenType::CARET);
            break;
        case '<':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::LESS_EQUAL);
            else if (IsMatchCurCharAndStepOnce('<'))
            {
                if (IsMatchCurCharAndStepOnce('='))
                    AddToken(TokenType::LESS_LESS_EQUAL);
                else
                    AddToken(TokenType::LESS_LESS);
            }
            else
                AddToken(TokenType::LESS);
            break;
        case '>':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::GREATER_EQUAL);
            else if (IsMatchCurCharAndStepOnce('>'))
            {
                if (IsMatchCurCharAndStepOnce('='))
                    AddToken(TokenType::GREATER_GREATER_EQUAL);
                else
                    AddToken(TokenType::GREATER_GREATER);
            }
            else
                AddToken(TokenType::GREATER);
            break;
        case '=':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TokenType::EQUAL_EQUAL);
            else
                AddToken(TokenType::EQUAL);
            break;
        case '?':
            AddToken(TokenType::QUESTION);
            break;
        default:
            if (IsNumber(c))
                Number();
            else if (IsLetter(c))
                Identifier();
            else
            {
                auto literal= m_Source.substr(m_StartPos, m_CurPos - m_StartPos);
                std::cout << "Unknown literal:" << literal << std::endl;
                exit(1);
            }
            break;
        }
    }

    void Lexer::ResetStatus()
    {
        m_StartPos = m_CurPos = 0;
        m_Line = 1;
        std::vector<Token>().swap(m_Tokens);
    }

    bool Lexer::IsMatchCurChar(char c)
    {
        return GetCurChar() == c;
    }
    bool Lexer::IsMatchCurCharAndStepOnce(char c)
    {
        bool result = GetCurChar() == c;
        if (result)
            m_CurPos++;
        return result;
    }

    bool Lexer::IsMatchNextChar(char c)
    {
        return GetNextChar() == c;
    }
    bool Lexer::IsMatchNextCharAndStepOnce(char c)
    {
        bool result = GetNextChar() == c;
        if (result)
            m_CurPos++;
        return result;
    }

    char Lexer::GetNextCharAndStepOnce()
    {
        if (m_CurPos + 1 < m_Source.size())
            return m_Source[++m_CurPos];
        return '\0';
    }
    char Lexer::GetNextChar()
    {
        if (m_CurPos + 1 < m_Source.size())
            return m_Source[m_CurPos + 1];
        return '\0';
    }
    char Lexer::GetCurCharAndStepOnce()
    {
        if (!IsAtEnd())
            return m_Source[m_CurPos++];
        return '\0';
    }

    char Lexer::GetCurChar()
    {
        if (!IsAtEnd())
            return m_Source[m_CurPos];
        return '\0';
    }

    void Lexer::AddToken(TokenType type)
    {
        auto literal = m_Source.substr(m_StartPos, m_CurPos - m_StartPos);
        m_Tokens.emplace_back(Token(type, literal, m_Line));
    }
    void Lexer::AddToken(TokenType type, std::string_view literal)
    {
        m_Tokens.emplace_back(Token(type, literal, m_Line));
    }

    bool Lexer::IsAtEnd()
    {
        return m_CurPos >= m_Source.size();
    }

    bool Lexer::IsNumber(char c)
    {
        return c >= '0' && c <= '9';
    }
    bool Lexer::IsLetter(char c)
    {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
    }
    bool Lexer::IsLetterOrNumber(char c)
    {
        return IsLetter(c) || IsNumber(c);
    }

    void Lexer::Number()
    {
        while (IsNumber(GetCurChar()))
            GetCurCharAndStepOnce();

        if (IsMatchCurCharAndStepOnce('.'))
        {
            if (IsNumber(GetCurChar()))
                while (IsNumber(GetCurChar()))
                    GetCurCharAndStepOnce();
            else
                Assert("[line " + std::to_string(m_Line) + "]:Number cannot end with '.'");
        }

        AddToken(TokenType::NUMBER);
    }

    void Lexer::Identifier()
    {
        while (IsLetterOrNumber(GetCurChar()))
            GetCurCharAndStepOnce();

        std::string literal = m_Source.substr(m_StartPos, m_CurPos - m_StartPos);

        bool isKeyWord = false;
        for (const auto &[key, value] : keywords)
            if (key.compare(literal) == 0)
            {
                AddToken(value, literal);
                isKeyWord = true;
                break;
            }

        if (!isKeyWord)
            AddToken(TokenType::IDENTIFIER, literal);
    }

    void Lexer::String()
    {
        while (!IsMatchCurChar('\"') && !IsAtEnd())
        {
            if (IsMatchCurChar('\n'))
                m_Line++;
            GetCurCharAndStepOnce();
        }

        if (IsAtEnd())
            std::cout << "[line " << m_Line << "]:Uniterminated string." << std::endl;

        GetCurCharAndStepOnce(); //eat the second '\"'

        AddToken(TokenType::STRING, m_Source.substr(m_StartPos + 1, m_CurPos - m_StartPos - 2));
    }
}