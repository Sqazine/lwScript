#include <string>
#include <string_view>
#include "liblwscript/lwScript.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)
#endif

lwscript::Lexer *gLexer{nullptr};
lwscript::Parser *gParser{nullptr};
lwscript::Compiler *gCompiler{nullptr};
lwscript::VM *gVm{nullptr};

std::string_view gSourceFilePath;

bool gIsSerializeBinaryChunk{false};
std::string_view gSerializeBinaryFilePath;

int32_t PrintVersion()
{
	lwscript::Logger::Info(TEXT(LWSCRIPT_VERSION));
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
#ifndef NDEBUG
	lwscript::Logger::Println(TEXT("{}"), stmt->ToString());
#endif
	auto mainFunc = gCompiler->Compile(stmt);

#ifndef NDEBUG
	auto str = mainFunc->ToStringWithChunk();
	lwscript::Logger::Println(TEXT("{}"), str);
#endif

	if (gIsSerializeBinaryChunk)
	{
		auto data = mainFunc->chunk.Serialize();
		lwscript::WriteBinaryFile(gSerializeBinaryFilePath, data);
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

int32_t PrintUsage()
{
	lwscript::Logger::Info(TEXT("Usage: lwscript [option]:"));
	lwscript::Logger::Info(TEXT("-h or --help:show usage info."));
	lwscript::Logger::Info(TEXT("-v or --version:show current lwscript version"));
	lwscript::Logger::Info(TEXT("-s or --serialize: serialize source file as bytecode binary file"));
	lwscript::Logger::Info(TEXT("-f or --file:run source file with a valid file path,like : lwscript -f examples/array.cd."));
	lwscript::Logger::Info(TEXT("-nfc or --no-function-cache:cache function execute result."));
	lwscript::Logger::Info(TEXT("-ncf or --no-constant-fold:use constant fold optimize on parsing stage."));
	return EXIT_FAILURE;
}

int main(int argc, const char *argv[])
{
#if defined(_WIN32) || defined(_WIN64)
	system("chcp 65001");
#endif
	for (size_t i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0)
		{
			if (i + 1 < argc)
				gSourceFilePath = argv[++i];
			else
				return PrintUsage();
		}

		if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--serialize") == 0)
		{
			if (i + 1 < argc)
			{
				gIsSerializeBinaryChunk = true;
				gSerializeBinaryFilePath = argv[++i];
			}
			else
				return PrintUsage();
		}

		if (strcmp(argv[i], "-nfc") == 0 || strcmp(argv[i], "--no-function-cache") == 0)
			lwscript::Config::GetInstance()->SetIsUseFunctionCache(false);

		if (strcmp(argv[i], "-ncf") == 0 || strcmp(argv[i], "--no-constant-fold") == 0)
			lwscript::Config::GetInstance()->SetIsUseConstantFold(false);

		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			return PrintUsage();

		if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
			return PrintVersion();
	}

	gLexer = new lwscript::Lexer();
	gParser = new lwscript::Parser();
	gCompiler = new lwscript::Compiler();
	gVm = new lwscript::VM();

	if (!gSourceFilePath.empty())
		RunFile(gSourceFilePath);
	else
		Repl();

	SAFE_DELETE(gLexer);
	SAFE_DELETE(gParser);
	SAFE_DELETE(gCompiler);
	SAFE_DELETE(gVm);

	return 0;
}