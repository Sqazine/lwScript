#include <string>
#include <string_view>
#include <codecvt>
#include "liblwScript/lwScript.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)
#endif

void Repl()
{
	std::wstring line;
	lws::Lexer lexer;
	lws::Parser parser;
	lws::Compiler compiler;
	lws::VM vm;

	std::wcout << L">> ";
	while (getline(std::wcin, line))
	{
		if (line == L"clear")
		{
			compiler.ResetStatus();
		}
		else
		{
			auto tokens = lexer.ScanTokens(line);
#ifdef _DEBUG
			for (const auto &token : tokens)
				lws::OutputToConsole(L"{}", *token);
#endif
			auto stmt = parser.Parse(tokens);
#ifdef _DEBUG
			lws::OutputToConsole(L"{}", stmt->ToString());
#endif
			auto mainFunc = compiler.Compile(stmt);
#ifdef _DEBUG
			lws::OutputToConsole(L"{}", mainFunc->ToString());
#endif
			vm.Run(mainFunc);
		}
		std::wcout << L">> ";
	}
}

void RunFile(std::string_view path)
{
	std::wstring content = lws::ReadFile(path);
	lws::Lexer lexer;
	lws::Parser parser;
	lws::Compiler compiler;
	lws::VM vm;

	auto tokens = lexer.ScanTokens(content);
#ifdef _DEBUG
	for (const auto &token : tokens)
		lws::OutputToConsole(L"{}", *token);
#endif
	auto stmt = parser.Parse(tokens);
#ifdef _DEBUG
	lws::OutputToConsole(L"{}", stmt->ToString());
#endif
	auto mainFunc = compiler.Compile(stmt);
#ifdef _DEBUG
	lws::OutputToConsole(L"{}", mainFunc->ToString());
#endif

	vm.Run(mainFunc);
}

int main(int argc, const char *argv[])
{
#if defined(_WIN32) || defined(_WIN64)
	system("chcp 65001");
#endif

	 if (argc == 2)
	 	RunFile(argv[1]);
	 else if (argc == 1)
	 	Repl();
	 else
	 	lws::OutputToConsole(L"Usage: lwScript [filepath]");

	return 0;
}