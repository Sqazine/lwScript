#pragma once
#include <array>
#include <cstdint>
#include <stack>
#include "Frame.h"
namespace lwScript
{
	  struct Object;

     class Environment
    {
    public:
        Environment();
        Environment(Environment* upEnvironment);
        ~Environment();

        void DefineVariable(std::string_view name,Object* value);
        void AssignVariable(std::string_view name,Object* value);
        Object* GetVariable(std::string_view name);

        Environment* GetUpEnvironment();
        const std::unordered_map<std::string,Object*>& GetValues() const;
    private:
        std::unordered_map<std::string,Object*> m_Values;
        Environment* m_UpEnvironment;
    };

	class VM
	{
	public:
		VM();
		~VM();

		void Execute(const Frame& frame);

	private:
		void ResetStatus();

		void Push(Object* object);
		Object* Pop();
		uint8_t ip;
		uint8_t sp;
		std::array<Object*, 64> m_Stack;

		Environment* m_Environment;

		Frame m_RootFrame;
	};
}