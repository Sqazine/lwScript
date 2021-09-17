#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
namespace lwScript
{
	enum class ObjectType
	{
		NUM,
		STR,
		BOOL,
		NIL,
		ARRAY,
	};

	struct Object
	{
		Object() {}
		virtual ~Object() {}

		virtual std::string Stringify() = 0;
		virtual ObjectType Type() = 0;
	};

	struct NumObject : public Object
	{
		NumObject() {}
		NumObject(double value) : value(value) {}
		~NumObject() {}

		std::string Stringify() override { return std::to_string(value); }
		ObjectType Type() override { return ObjectType::NUM; }

		double value;
	};

	struct StrObject : public Object
	{
		StrObject() {}
		StrObject(std::string_view value) : value(value) {}
		~StrObject() {}

		std::string Stringify() override { return value; }
		ObjectType Type() override { return ObjectType::STR; }


		std::string value;
	};

	struct BoolObject : public Object
	{
		BoolObject() {}
		BoolObject(bool value) : value(value) {}
		~BoolObject() {}

		std::string Stringify() override { return value ? "true" : "false"; }
		ObjectType Type() override { return ObjectType::BOOL; }


		bool value;
	};

	struct NilObject : public Object
	{
		NilObject() {}
		~NilObject() {}

		std::string Stringify() override { return "nil"; }
		ObjectType Type() override { return ObjectType::NIL; }

	};

	struct ArrayObject : public Object
	{
		ArrayObject() {}
		ArrayObject(const std::vector<Object*>& elements) : elements(elements) {}
		~ArrayObject() {}

		std::string Stringify() override
		{
			std::string result = "[";
			if (!elements.empty())
			{
				for (const auto& e : elements)
					result += e->Stringify() + ",";
				result = result.substr(0, result.size() - 1);
			}
			result += "]";
			return result;
		}
		ObjectType Type() override { return ObjectType::ARRAY; }

		std::vector<Object*> elements;
	};
}
