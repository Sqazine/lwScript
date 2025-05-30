#include <string>
#include <string_view>
#include "lwScript.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)
#endif

lwScript::Lexer *gLexer{nullptr};
lwScript::Parser *gParser{nullptr};

lwScript::AstPassManager *gAstPassManager;

lwScript::Compiler *gCompiler{nullptr};
lwScript::VM *gVm{nullptr};

struct Config
{
	std::string_view sourceFilePath;
	bool isSerializeBinaryChunk{false};
	std::string_view serializeBinaryFilePath;
} gConfig;

int32_t PrintVersion()
{
	LWS_LOG_INFO(LWS_VERSION);
	return EXIT_FAILURE;
}

int32_t PrintUsage()
{
	LWS_LOG_INFO(TEXT("Usage: lwScript [option]:"));
	LWS_LOG_INFO(TEXT("-h or --help:show usage info."));
	LWS_LOG_INFO(TEXT("-v or --version:show current lwScript version"));
	LWS_LOG_INFO(TEXT("-s or --serialize: serialize source file as bytecode binary file"));
	LWS_LOG_INFO(TEXT("-f or --file:run source file with a valid file path,like : lwScript -f examples/array.cd."));
	return EXIT_FAILURE;
}

void Run(STRING_VIEW content)
{
	auto tokens = gLexer->ScanTokens(content);
#ifndef NDEBUG
	for (const auto &token : tokens)
		lwScript::Logger::Println(TEXT("{}"), *token);
#endif
	auto stmt = gParser->Parse(tokens);

	gAstPassManager->Execute(stmt);

#ifndef NDEBUG
	lwScript::Logger::Println(TEXT("{}"), stmt->ToString());
#endif
	auto mainFunc = gCompiler->Compile(stmt);

#ifndef NDEBUG
	auto str = mainFunc->ToStringWithChunk();
	lwScript::Logger::Println(TEXT("{}"), str);
#endif

	if (gConfig.isSerializeBinaryChunk)
	{
		auto data = mainFunc->chunk.Serialize();
		lwScript::WriteBinaryFile(gConfig.serializeBinaryFilePath, data);
	}
	else
	{
		gVm->Run(mainFunc);
	}
}

void Repl()
{
	STRING line;
	STRING allLines;

	PrintVersion();

	lwScript::Logger::Print(TEXT(">> "));
	while (getline(CIN, line))
	{
		allLines += line;
		if (line == TEXT("clear"))
			allLines = TEXT("");
		else
			Run(allLines);
		lwScript::Logger::Print(TEXT(">> "));
	}
}

void RunFile(std::string_view path)
{
	STRING content = lwScript::ReadFile(path);
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

	gLexer = new lwScript::Lexer();
	gParser = new lwScript::Parser();
	gAstPassManager = new lwScript::AstPassManager();
	gCompiler = new lwScript::Compiler();
	gVm = new lwScript::VM();

	gAstPassManager
#ifdef LWS_CONSTANT_FOLD_OPT
		->Add<lwScript::ConstantFoldPass>()
#endif
		->Add<lwScript::TypeCheckPass>()
		->Add<lwScript::SyntaxCheckPass>();

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