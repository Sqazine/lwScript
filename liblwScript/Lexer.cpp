#include "Lexer.h"
namespace lws
{
    static std::unordered_map<std::string, TokenType> keywords =
        {
            {"let", TOKEN_LET},
            {"if", TOKEN_IF},
            {"else", TOKEN_ELSE},
            {"true", TOKEN_TRUE},
            {"false", TOKEN_FALSE},
            {"null", TOKEN_NULL},
            {"while", TOKEN_WHILE},
            {"for", TOKEN_FOR},
            {"fn", TOKEN_FUNCTION},
            {"field", TOKEN_FIELD},
            {"return", TOKEN_RETURN},
            {"static", TOKEN_STATIC},
            {"const", TOKEN_CONST},
            {"break", TOKEN_BREAK},
            {"continue", TOKEN_CONTINUE},
    };

    Lexer::Lexer()
    {
        ResetStatus();
    }
    Lexer::~Lexer()
    {
    }

    const std::vector<Token> &Lexer::ScanTokens(std::string_view src)
    {
        ResetStatus();
        mSource = src;
        while (!IsAtEnd())
        {
            mStartPos = mCurPos;
            ScanToken();
        }

        AddToken(TOKEN_END, "EOF");

        return mTokens;
    }
    void Lexer::ScanToken()
    {
        char c = GetCurCharAndStepOnce();

        switch (c)
        {
        case '(':
            AddToken(TOKEN_LPAREN);
            break;
        case ')':
            AddToken(TOKEN_RPAREN);
            break;
        case '[':
            AddToken(TOKEN_LBRACKET);
            break;
        case ']':
            AddToken(TOKEN_RBRACKET);
            break;
        case '{':
            AddToken(TOKEN_LBRACE);
            break;
        case '}':
            AddToken(TOKEN_RBRACE);
            break;
        case '.':
            AddToken(TOKEN_DOT);
            break;
        case ',':
            AddToken(TOKEN_COMMA);
            break;
        case ':':
            AddToken(TOKEN_COLON);
            break;
        case ';':
            AddToken(TOKEN_SEMICOLON);
            break;
        case '\"':
            String();
            break;
        case ' ':
        case '\t':
        case '\r':
            break;
        case '\n':
            mLine++;
            break;
        case '+':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_PLUS_EQUAL);
            else
                AddToken(TOKEN_PLUS);
            break;
        case '-':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_MINUS_EQUAL);
            else
                AddToken(TOKEN_MINUS);
            break;
        case '*':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_ASTERISK_EQUAL);
            else
                AddToken(TOKEN_ASTERISK);
            break;
        case '/':
            if (IsMatchCurCharAndStepOnce('/'))
            {
                while (!IsMatchCurChar('\n') && !IsAtEnd())
                    GetCurCharAndStepOnce();
                mLine++;
                break;
            }
            else if (IsMatchCurCharAndStepOnce('*'))
            {
                while (!IsMatchCurChar('*') && !IsMatchNextChar('/') && !IsAtEnd())
                {
                    if (IsMatchCurChar('\n'))
                        mLine++;
                    GetCurCharAndStepOnce();
                }
                GetCurCharAndStepOnce(); //eat '*'
                GetCurCharAndStepOnce(); // eat '/'
            }
            else if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_SLASH_EQUAL);
            else
                AddToken(TOKEN_SLASH);
            break;
        case '%':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_PERCENT_EQUAL);
            AddToken(TOKEN_PERCENT);
            break;
        case '~':
            AddToken(TOKEN_TILDE);
            break;
        case '!':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_BANG_EQUAL);
            else
                AddToken(TOKEN_BANG);
            break;
        case '&':
            if (IsMatchCurCharAndStepOnce('&'))
                AddToken(TOKEN_AMPERSAND_AMPERSAND);
            else if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_AMPERSAND_EQUAL);
            else
                AddToken(TOKEN_AMPERSAND);
            break;
        case '|':
            if (IsMatchCurCharAndStepOnce('|'))
                AddToken(TOKEN_VBAR_VBAR);
            else if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_VBAR_EQUAL);
            else
                AddToken(TOKEN_VBAR);
            break;
        case '^':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_CARET_EQUAL);
            else
                AddToken(TOKEN_CARET);
            break;
        case '<':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_LESS_EQUAL);
            else if (IsMatchCurCharAndStepOnce('<'))
            {
                if (IsMatchCurCharAndStepOnce('='))
                    AddToken(TOKEN_LESS_LESS_EQUAL);
                else
                    AddToken(TOKEN_LESS_LESS);
            }
            else
                AddToken(TOKEN_LESS);
            break;
        case '>':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_GREATER_EQUAL);
            else if (IsMatchCurCharAndStepOnce('>'))
            {
                if (IsMatchCurCharAndStepOnce('='))
                    AddToken(TOKEN_GREATER_GREATER_EQUAL);
                else
                    AddToken(TOKEN_GREATER_GREATER);
            }
            else
                AddToken(TOKEN_GREATER);
            break;
        case '=':
            if (IsMatchCurCharAndStepOnce('='))
                AddToken(TOKEN_EQUAL_EQUAL);
            else
                AddToken(TOKEN_EQUAL);
            break;
        case '?':
            AddToken(TOKEN_QUESTION);
            break;
        default:
            if (IsNumber(c))
                Number();
            else if (IsLetter(c))
                Identifier();
            else
            {
                auto literal = mSource.substr(mStartPos, mCurPos - mStartPos);
                std::cout << "Unknown literal:" << literal << std::endl;
                exit(1);
            }
            break;
        }
    }

    void Lexer::ResetStatus()
    {
        mStartPos = mCurPos = 0;
        mLine = 1;
        std::vector<Token>().swap(mTokens);
    }

    bool Lexer::IsMatchCurChar(char c)
    {
        return GetCurChar() == c;
    }
    bool Lexer::IsMatchCurCharAndStepOnce(char c)
    {
        bool result = GetCurChar() == c;
        if (result)
            mCurPos++;
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
            mCurPos++;
        return result;
    }

    char Lexer::GetNextCharAndStepOnce()
    {
        if (mCurPos + 1 < mSource.size())
            return mSource[++mCurPos];
        return '\0';
    }
    char Lexer::GetNextChar()
    {
        if (mCurPos + 1 < mSource.size())
            return mSource[mCurPos + 1];
        return '\0';
    }
    char Lexer::GetCurCharAndStepOnce()
    {
        if (!IsAtEnd())
            return mSource[mCurPos++];
        return '\0';
    }

    char Lexer::GetCurChar()
    {
        if (!IsAtEnd())
            return mSource[mCurPos];
        return '\0';
    }

    void Lexer::AddToken(TokenType type)
    {
        auto literal = mSource.substr(mStartPos, mCurPos - mStartPos);
        mTokens.emplace_back(Token(type, literal, mLine));
    }
    void Lexer::AddToken(TokenType type, std::string_view literal)
    {
        mTokens.emplace_back(Token(type, literal, mLine));
    }

    bool Lexer::IsAtEnd()
    {
        return mCurPos >= mSource.size();
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
                Assert("[line " + std::to_string(mLine) + "]:Number cannot end with '.'");
        }

        AddToken(TOKEN_NUMBER);
    }

    void Lexer::Identifier()
    {
        while (IsLetterOrNumber(GetCurChar()))
            GetCurCharAndStepOnce();

        std::string literal = mSource.substr(mStartPos, mCurPos - mStartPos);

        bool isKeyWord = false;
        for (const auto &[key, value] : keywords)
            if (key.compare(literal) == 0)
            {
                AddToken(value, literal);
                isKeyWord = true;
                break;
            }

        if (!isKeyWord)
            AddToken(TOKEN_IDENTIFIER, literal);
    }

    void Lexer::String()
    {
        while (!IsMatchCurChar('\"') && !IsAtEnd())
        {
            if (IsMatchCurChar('\n'))
                mLine++;
            GetCurCharAndStepOnce();
        }

        if (IsAtEnd())
            std::cout << "[line " << mLine << "]:Uniterminated string." << std::endl;

        GetCurCharAndStepOnce(); //eat the second '\"'

        AddToken(TOKEN_STRING, mSource.substr(mStartPos + 1, mCurPos - mStartPos - 2));
    }
}