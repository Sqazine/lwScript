#include <string>
#include <string_view>
#include "lwScript/lwScript.h"

void Repl()
{
	std::string line;
	lws::Lexer lexer;
	lws::Parser parser;
	lws::Compiler compiler;
	lws::VM vm;
	lws::Frame frame;
	std::cout << "> ";
	while (getline(std::cin, line))
	{
		if (line == "clear")
			compiler.ResetStatus();
		else
		{
			auto tokens = lexer.ScanTokens(line);
#ifdef _DEBUG
			for (const auto &token : tokens)
				std::cout << token << std::endl;
#endif
			auto stmt = parser.Parse(tokens);
#ifdef _DEBUG
			std::cout << stmt->Stringify() << std::endl;
#endif
			frame = compiler.Compile(stmt);
#ifdef _DEBUG
			std::cout << frame.Stringify() << std::endl;
#endif
			vm.ResetStatus();
			vm.Execute(frame);
		}
		std::cout << "> ";
	}
}

void RunFile(std::string path)
{
	std::string content = lws::ReadFile(path);
	lws::Lexer lexer;
	lws::Parser parser;
	lws::Compiler compiler;
	lws::VM vm;
	lws::Frame frame;

	auto tokens = lexer.ScanTokens(content);
#ifdef _DEBUG
	for (const auto &token : tokens)
		std::cout << token << std::endl;
#endif
	auto stmt = parser.Parse(tokens);
#ifdef _DEBUG
	std::cout << stmt->Stringify() << std::endl;
#endif
	frame = compiler.Compile(stmt);
#ifdef _DEBUG
	std::cout << frame.Stringify() << std::endl;
#endif
	vm.Execute(frame);
}

int main(int argc, char **argv)
{
	if (argc == 2)
		RunFile(argv[1]);
	else if (argc == 1)
		Repl();
	else
		std::cout << "Usage: repl [filepath]" << std::endl;
	return 0;
}