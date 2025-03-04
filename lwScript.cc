#include <string>
#include <string_view>
#include "lwscript.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)
#endif

lwscript::Lexer *gLexer{nullptr};
lwscript::Parser *gParser{nullptr};

lwscript::AstPassManager *gAstPassManager;

lwscript::Compiler *gCompiler{nullptr};
lwscript::VM *gVm{nullptr};

struct Config
{
	std::string_view sourceFilePath;
	bool isSerializeBinaryChunk{false};
	std::string_view serializeBinaryFilePath;
} gConfig;

int32_t PrintVersion()
{
	LW_LOG_INFO(LWSCRIPT_VERSION);
	return EXIT_FAILURE;
}

int32_t PrintUsage()
{
	LW_LOG_INFO(TEXT("Usage: lwscript [option]:"));
	LW_LOG_INFO(TEXT("-h or --help:show usage info."));
	LW_LOG_INFO(TEXT("-v or --version:show current lwscript version"));
	LW_LOG_INFO(TEXT("-s or --serialize: serialize source file as bytecode binary file"));
	LW_LOG_INFO(TEXT("-f or --file:run source file with a valid file path,like : lwscript -f examples/array.cd."));
	return EXIT_FAILURE;
}

void Run(STD_STRING_VIEW content)
{
	auto tokens = gLexer->ScanTokens(content);
#ifndef NDEBUG
	for (const auto &token : tokens)
		lwscript::Logger::Println(TEXT("{}"), *token);
#endif
	auto stmt = gParser->Parse(tokens);

	gAstPassManager->Execute(stmt);

#ifndef NDEBUG
	lwscript::Logger::Println(TEXT("{}"), stmt->ToString());
#endif
	auto mainFunc = gCompiler->Compile(stmt);

#ifndef NDEBUG
	auto str = mainFunc->ToStringWithChunk();
	lwscript::Logger::Println(TEXT("{}"), str);
#endif

	if (gConfig.isSerializeBinaryChunk)
	{
		auto data = mainFunc->chunk.Serialize();
		lwscript::WriteBinaryFile(gConfig.serializeBinaryFilePath, data);
	}
	else
	{
		gVm->Run(mainFunc);
	}
}

void Repl()
{
	STD_STRING line;
	STD_STRING allLines;

	PrintVersion();

	lwscript::Logger::Print(TEXT(">> "));
	while (getline(CIN, line))
	{
		allLines += line;
		if (line == TEXT("clear"))
			allLines = TEXT("");
		else
			Run(allLines);
		lwscript::Logger::Print(TEXT(">> "));
	}
}

void RunFile(std::string_view path)
{
	STD_STRING content = lwscript::ReadFile(path);
	Run(content);
}

int32_t ParseArgs(int32_t argc, const char *argv[])
{
	for (size_t i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0)
		{
			if (i + 1 < argc)
				gConfig.sourceFilePath = argv[++i];
			else
				return PrintUsage();
		}

		if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--serialize") == 0)
		{
			if (i + 1 < argc)
			{
				gConfig.isSerializeBinaryChunk = true;
				gConfig.serializeBinaryFilePath = argv[++i];
			}
			else
				return PrintUsage();
		}

		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			return PrintUsage();

		if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
			return PrintVersion();
	}

	return EXIT_SUCCESS;
}

int32_t main(int32_t argc, const char *argv[])
{
#if defined(_WIN32) || defined(_WIN64)
	system("chcp 65001");
#endif
	if (ParseArgs(argc, argv) == EXIT_FAILURE)
		return EXIT_FAILURE;

	gLexer = new lwscript::Lexer();
	gParser = new lwscript::Parser();
	gAstPassManager = new lwscript::AstPassManager();
	gCompiler = new lwscript::Compiler();
	gVm = new lwscript::VM();

	gAstPassManager
#ifdef LWSCRIPT_CONSTANT_FOLD_OPT
		->Add<lwscript::ConstantFoldPass>()
#endif
		->Add<lwscript::TypeCheckPass>()
		->Add<lwscript::SyntaxCheckPass>();

	if (!gConfig.sourceFilePath.empty())
		RunFile(gConfig.sourceFilePath);
	else
		Repl();

	SAFE_DELETE(gLexer);
	SAFE_DELETE(gParser);
	SAFE_DELETE(gAstPassManager);
	SAFE_DELETE(gCompiler);
	SAFE_DELETE(gVm);

	return EXIT_SUCCESS;
}