#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <sstream>
#include "lwScript/lwScript.h"

std::string ReadFile(std::string_view path)
{
	std::fstream file;
	file.open(path.data(), std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		std::cout << "failed to open file:" << path << std::endl;
		exit(1);
	}

	std::stringstream sstream;
	sstream << file.rdbuf();
	return sstream.str();
}

void Repl()
{
	std::string line;
	lwScript::Lexer lexer;
	lwScript::Parser parser;
	lwScript::Compiler compiler;
	lwScript::VM vm;
	lwScript::Frame frame;
	std::cout << "> ";
	while (getline(std::cin, line))
	{
		if (line == "clear")
			compiler.ResetStatus();
		else {
			auto tokens = lexer.ScanTokens(line);
#ifdef _DEBUG
			for (const auto& token : tokens)
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
		std::cout << "> ";
	}
}

void RunFile(std::string path)
{
	std::string content = ReadFile(path);
	lwScript::Lexer lexer;
	lwScript::Parser parser;
	lwScript::Compiler compiler;
	lwScript::VM vm;
	lwScript::Frame frame;

	auto tokens = lexer.ScanTokens(content);
#ifdef _DEBUG
	for (const auto& token : tokens)
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

int main(int argc, char** argv)
{
	if (argc == 2)
		RunFile(argv[1]);
	else if (argc == 1)
		Repl();
	else
		std::cout << "Usage: repl [filepath]" << std::endl;
	return 0;
}