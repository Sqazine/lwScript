#include <string>
#include <string_view>
#include <codecvt>
#include "lwScript.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)
#endif

void Repl()
{
	std::wstring line;
	std::wstring allLines;
	lwscript::Lexer lexer;
	lwscript::Parser parser;
	lwscript::Compiler compiler;
	lwscript::VM vm;

	lwscript::Print(L">> ");
	while (getline(std::wcin, line))
	{
		allLines += line;
		if (line == L"clear")
		{
			allLines = L"";
		}
		else
		{
			auto tokens = lexer.ScanTokens(allLines);
#ifdef _DEBUG
			for (const auto &token : tokens)
				lwscript::Println(L"{}", *token);
#endif
			auto stmt = parser.Parse(tokens);
#ifdef _DEBUG
			lwscript::Println(L"{}", stmt->ToString());
#endif
			auto mainFunc = compiler.Compile(stmt);
#ifdef _DEBUG
			lwscript::Println(L"{}", mainFunc->ToString());
#endif
			vm.Run(mainFunc);
		}
		lwscript::Println(L">> ");
	}
}

void RunFile(std::string_view path)
{
	std::wstring content = lwscript::ReadFile(path);
	lwscript::Lexer lexer;
	lwscript::Parser parser;
	lwscript::Compiler compiler;
	lwscript::VM vm;

	auto tokens = lexer.ScanTokens(content);
#ifdef _DEBUG
	for (const auto &token : tokens)
		lwscript::Println(L"{}", *token);
#endif
	auto stmt = parser.Parse(tokens);
#ifdef _DEBUG
	lwscript::Println(L"{}", stmt->ToString());
#endif
	auto mainFunc = compiler.Compile(stmt);
#ifdef _DEBUG
	lwscript::Println(L"{}", mainFunc->ToString());
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
		lwscript::Println(L"Usage: lwScript [filepath]");

	return 0;
}