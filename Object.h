#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>

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
	Object() : marked(false), next(nullptr) {}
	virtual ~Object() {}

	virtual std::string Stringify() = 0;
	virtual ObjectType Type() = 0;
	virtual void Mark() = 0;
	virtual void UnMark() = 0;

	bool marked;
	Object *next;
};

struct NumObject : public Object
{
	NumObject() : value(0.0) {}
	NumObject(double value) : value(value) {}
	~NumObject() {}

	std::string Stringify() override { return std::to_string(value); }
	ObjectType Type() override { return ObjectType::NUM; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }

	double value;
};

struct StrObject : public Object
{
	StrObject() {}
	StrObject(std::string_view value) : value(value) {}
	~StrObject() {}

	std::string Stringify() override { return value; }
	ObjectType Type() override { return ObjectType::STR; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }

	std::string value;
};

struct BoolObject : public Object
{
	BoolObject() : value(false) {}
	BoolObject(bool value) : value(value) {}
	~BoolObject() {}

	std::string Stringify() override { return value ? "true" : "false"; }
	ObjectType Type() override { return ObjectType::BOOL; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }

	bool value;
};

struct NilObject : public Object
{
	NilObject() {}
	~NilObject() {}

	std::string Stringify() override { return "nil"; }
	ObjectType Type() override { return ObjectType::NIL; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }
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
	void Mark() override
	{
		if (marked)
			return;
		marked = true;

		for (const auto &e : elements)
			e->Mark();
	}
	void UnMark() override
	{
		if (!marked)
			return;
		marked = false;

		for (const auto &e : elements)
			e->UnMark();
	}

	std::vector<Object *> elements;
};

struct FunctionObject : public Object
{
	FunctionObject() : frameIndex(0) {}
	FunctionObject(int64_t frameIndex) : frameIndex(frameIndex) {}
	~FunctionObject() {}

	std::string Stringify() override { return "function:" + std::to_string(frameIndex); }
	ObjectType Type() override { return ObjectType::FUNCTION; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }

	int64_t frameIndex;
};

struct NativeFunctionObject : public Object
{
	NativeFunctionObject() {}
	NativeFunctionObject(std::string_view name, std::function<Object *(std::vector<Object *>)> function) : name(name), function(function) {}
	~NativeFunctionObject() {}

	std::string Stringify() override { return "native function:" + name; }
	ObjectType Type() override { return ObjectType::NATIVEFUNCTION; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }

	std::string name;
	std::function<Object *(std::vector<Object *>)> function;
};

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