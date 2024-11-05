#include <string>
#include <string_view>
#include <codecvt>
#include "liblwscript/lwScript.h"

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
#ifndef NDEBUG
	for (const auto &token : tokens)
		lwscript::Println(L"{}", *token);
#endif
	auto stmt = gParser->Parse(tokens);
#ifndef NDEBUG
	lwscript::Println(L"{}", stmt->ToString());
#endif
	auto mainFunc = gCompiler->Compile(stmt);
#ifndef NDEBUG
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

int32_t PrintUsage()
{
	std::cout << "Usage: lwscript [option]:" << std::endl;
	std::cout << "-h or --help:show usage info." << std::endl;
	std::cout << "-f or --file:run source file with a valid file path,like : lwscript -f examples/array.cd." << std::endl;
	std::cout << "-fc or --function-cache:cache function execute result." << std::endl;
	return EXIT_FAILURE;
}

int main(int argc, const char *argv[])
{
#if defined(_WIN32) || defined(_WIN64)
	system("chcp 65001");
#endif
	std::string_view sourceFilePath;
	for (size_t i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0)
		{
			if (i + 1 < argc)
				sourceFilePath = argv[++i];
			else
				return PrintUsage();
		}

		if (strcmp(argv[i], "-fc") == 0 || strcmp(argv[i], "--function-cache") == 0)
			lwscript::Config::GetInstance()->SetIsUseFunctionCache(true);

		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			return PrintUsage();
	}

	gLexer = new lwscript::Lexer();
	gParser = new lwscript::Parser();
	gCompiler = new lwscript::Compiler();
	gVm = new lwscript::VM();

	if (!sourceFilePath.empty())
		RunFile(sourceFilePath);
	else
		Repl();

	SAFE_DELETE(gLexer);
	SAFE_DELETE(gParser);
	SAFE_DELETE(gCompiler);
	SAFE_DELETE(gVm);

	return 0;
}