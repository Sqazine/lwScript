#include <string>
#include <string_view>
#include <codecvt>
#include "lwScript.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)
#endif

lwscript::Lexer *gLexer = nullptr;
lwscript::Parser *gParser = nullptr;
lwscript::Compiler *gCompiler = nullptr;
lwscript::VM *gVm = nullptr;

void Run(std::wstring_view content)
{
	auto tokens = gLexer->ScanTokens(content);
#ifdef _DEBUG
	for (const auto &token : tokens)
		lwscript::Println(L"{}", *token);
#endif
	auto stmt = gParser->Parse(tokens);
#ifdef _DEBUG
	lwscript::Println(L"{}", stmt->ToString());
#endif
	auto mainFunc = gCompiler->Compile(stmt);
#ifdef _DEBUG
	auto str = mainFunc->ToStringWithChunk();
	lwscript::Println(L"{}", str);
#endif
	gVm->Run(mainFunc);
}

void Repl()
{
	std::wstring line;
	std::wstring allLines;
	

	lwscript::Print(L">> ");
	while (getline(std::wcin, line))
	{
		allLines += line;
		if (line == L"clear")
			allLines = L"";
		else
			Run(allLines);
		lwscript::Println(L">> ");
	}
}

void RunFile(std::string_view path)
{
	std::wstring content = lwscript::ReadFile(path);
	Run(content);
}

int main(int argc, const char *argv[])
{
#if defined(_WIN32) || defined(_WIN64)
	system("chcp 65001");
#endif

	gLexer = new lwscript::Lexer();
	gParser = new lwscript::Parser();
	gCompiler = new lwscript::Compiler();
	gVm = new lwscript::VM();

	if (argc == 2)
		RunFile(argv[1]);
	else if (argc == 1)
		Repl();
	else
		lwscript::Println(L"Usage: lwScript [filepath]");

	SAFE_DELETE(gLexer);
	SAFE_DELETE(gParser);
	SAFE_DELETE(gCompiler);
	SAFE_DELETE(gVm);

	return 0;
}