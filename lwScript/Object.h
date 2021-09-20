#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
namespace lws
{
	enum class ObjectType
	{
		NUM,
		STR,
		BOOL,
		NIL,
		ARRAY,
		FUNCTION,
		NATIVEFUNCTION,
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
		ArrayObject(const std::vector<Object *> &elements) : elements(elements) {}
		~ArrayObject() {}

		std::string Stringify() override
		{
			std::string result = "[";
			if (!elements.empty())
			{
				for (const auto &e : elements)
					result += e->Stringify() + ",";
				result = result.substr(0, result.size() - 1);
			}
			result += "]";
			return result;
		}
		ObjectType Type() override { return ObjectType::ARRAY; }

		std::vector<Object *> elements;
	};

	struct FunctionObject : public Object
	{
		FunctionObject() {}
		FunctionObject(int64_t frameIndex) : frameIndex(frameIndex) {}
		~FunctionObject() {}

		std::string Stringify() override { return "function:" + std::to_string(frameIndex); }
		ObjectType Type() override { return ObjectType::FUNCTION; }

		int64_t frameIndex;
	};

	struct NativeFunctionObject : public Object
	{
		NativeFunctionObject() {}
		NativeFunctionObject(std::string_view name, std::function<Object *(std::vector<Object *>)> function) : name(name), function(function) {}
		~NativeFunctionObject() {}

		std::string Stringify() override { return "native function:" + name; }
		ObjectType Type() override { return ObjectType::NATIVEFUNCTION; }

		std::string name;
		std::function<Object *(std::vector<Object *>)> function;
	};

	static BoolObject *trueObject = new BoolObject(true);
	static BoolObject *falseObject = new BoolObject(false);
	static NilObject *nilObject = new NilObject();

#define TO_NUM_OBJ(obj) ((NumObject *)obj)
#define TO_STR_OBJ(obj) ((StrObject *)obj)
#define TO_NIL_OBJ(obj) ((NilObject *)obj)
#define TO_BOOL_OBJ(obj) ((BoolObject *)obj)
#define TO_ARRAY_OBJ(obj) ((ArrayObject *)obj)
#define TO_FUNCTION_OBJ(obj) ((FunctionObject *)obj)
#define TO_NATIVE_FUNCTION_OBJ(obj) ((NativeFunctionObject *)obj)

#define IS_NUM_OBJ(obj) (obj->Type() == ObjectType::NUM)
#define IS_STR_OBJ(obj) (obj->Type() == ObjectType::STR)
#define IS_BOOL_OBJ(obj) (obj->Type() == ObjectType::BOOL)
#define IS_NIL_OBJ(obj) (obj->Type() == ObjectType::NIL)
#define IS_ARRAY_OBJ(obj) (obj->Type() == ObjectType::ARRAY)
#define IS_FUNCTION_OBJ(obj) (obj->Type() == ObjectType::FUNCTION)
#define IS_NATIVE_FUNCTION_OBJ(obj) (obj->Type() == ObjectType::NATIVEFUNCTION)
}
