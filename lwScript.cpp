#include <string>
#include <string_view>
#include "liblwscript/lwScript.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)
#endif

lwscript::Lexer *gLexer = nullptr;
lwscript::Parser *gParser = nullptr;
lwscript::Compiler *gCompiler = nullptr;
lwscript::VM *gVm = nullptr;

void Run(STD_STRING_VIEW content)
{
	auto tokens = gLexer->ScanTokens(content);
#ifndef NDEBUG
	for (const auto &token : tokens)
		lwscript::Logger::Println(TEXT("{}"), *token);
#endif
	auto stmt = gParser->Parse(tokens);
#ifndef NDEBUG
	lwscript::Logger::Println(TEXT("{}"), stmt->ToString());
#endif
	auto mainFunc = gCompiler->Compile(stmt);
#ifndef NDEBUG
	auto str = mainFunc->ToStringWithChunk();
	lwscript::Logger::Println(TEXT("{}"), str);
#endif
	gVm->Run(mainFunc);
}

void Repl()
{
	STD_STRING line;
	STD_STRING allLines;

	lwscript::Logger::Print(TEXT(">> "));
	while (getline(CIN, line))
	{
		allLines += line;
		if (line == TEXT("clear"))
			allLines = TEXT("");
		else
			Run(allLines);
		lwscript::Logger::Println(TEXT(">> "));
	}
}

void RunFile(std::string_view path)
{
	STD_STRING content = lwscript::ReadFile(path);
	Run(content);
}

int32_t PrintUsage()
{
	lwscript::Logger::Info(TEXT("Usage: lwscript [option]:"));
	lwscript::Logger::Info(TEXT("-h or --help:show usage info."));
	lwscript::Logger::Info(TEXT("-f or --file:run source file with a valid file path,like : lwscript -f examples/array.cd."));
	lwscript::Logger::Info(TEXT("-fc or --function-cache:cache function execute result."));
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