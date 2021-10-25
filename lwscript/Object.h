#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include "Context.h"
namespace lws
{
#define TO_INTEGER_OBJ(obj) ((IntegerObject *)obj)
#define TO_FLOATING_OBJ(obj) ((FloatingObject *)obj)
#define TO_STR_OBJ(obj) ((StrObject *)obj)
#define TO_NIL_OBJ(obj) ((NilObject *)obj)
#define TO_BOOL_OBJ(obj) ((BoolObject *)obj)
#define TO_ARRAY_OBJ(obj) ((ArrayObject *)obj)
#define TO_TABLE_OBJ(obj) ((TableObject *)obj)
#define TO_STRUCT_OBJ(obj) ((StructObject *)obj)
#define TO_REF_OBJ(obj) ((RefObject *)obj)

#define IS_INTEGER_OBJ(obj) (obj->Type() == ObjectType::INTEGER)
#define IS_FLOATING_OBJ(obj) (obj->Type() == ObjectType::FLOATING)
#define IS_STR_OBJ(obj) (obj->Type() == ObjectType::STR)
#define IS_BOOL_OBJ(obj) (obj->Type() == ObjectType::BOOL)
#define IS_NIL_OBJ(obj) (obj->Type() == ObjectType::NIL)
#define IS_ARRAY_OBJ(obj) (obj->Type() == ObjectType::ARRAY)
#define IS_TABLE_OBJ(obj) (obj->Type() == ObjectType::TABLE)
#define IS_STRUCT_OBJ(obj) (obj->Type() == ObjectType::STRUCT)
#define IS_REF_OBJ(obj) (obj->Type() == ObjectType::REF)

	enum class ObjectType
	{
		INTEGER,
		FLOATING,
		STR,
		BOOL,
		NIL,
		ARRAY,
		TABLE,
		STRUCT,
		REF
	};

	struct Object
	{
		Object() : marked(false), next(nullptr) {}
		virtual ~Object() {}

		virtual std::string Stringify() = 0;
		virtual ObjectType Type() = 0;
		virtual void Mark() = 0;
		virtual void UnMark() = 0;
		virtual bool IsEqualTo(Object *other) = 0;

		bool marked;
		Object *next;
	};

	struct IntegerObject : public Object
	{
		IntegerObject() : value(0) {}
		IntegerObject(int64_t value) : value(value) {}
		~IntegerObject() {}

		std::string Stringify() override { return std::to_string(value); }
		ObjectType Type() override { return ObjectType::INTEGER; }
		void Mark() override { marked = true; }
		void UnMark() override { marked = false; }
		bool IsEqualTo(Object *other) override
		{
			if (!IS_INTEGER_OBJ(other))
				return false;
			return value == TO_INTEGER_OBJ(other)->value;
		}

		int64_t value;
	};

	struct FloatingObject : public Object
	{
		FloatingObject() : value(0.0) {}
		FloatingObject(double value) : value(value) {}
		~FloatingObject() {}

		std::string Stringify() override { return std::to_string(value); }
		ObjectType Type() override { return ObjectType::FLOATING; }
		void Mark() override { marked = true; }
		void UnMark() override { marked = false; }
		bool IsEqualTo(Object *other) override
		{
			if (!IS_FLOATING_OBJ(other))
				return false;
			return value == TO_FLOATING_OBJ(other)->value;
		}

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
		bool IsEqualTo(Object *other) override
		{
			if (!IS_STR_OBJ(other))
				return false;
			return value == TO_STR_OBJ(other)->value;
		}

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
		bool IsEqualTo(Object *other) override
		{
			if (!IS_BOOL_OBJ(other))
				return false;
			return value == TO_BOOL_OBJ(other)->value;
		}

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
		bool IsEqualTo(Object *other) override
		{
			if (!IS_NIL_OBJ(other))
				return false;
			return true;
		}
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

		bool IsEqualTo(Object *other) override
		{
			if (!IS_ARRAY_OBJ(other))
				return false;

			ArrayObject *arrayOther = TO_ARRAY_OBJ(other);

			if (arrayOther->elements.size() != elements.size())
				return false;

			for (size_t i = 0; i < elements.size(); ++i)
				if (elements[i] != arrayOther->elements[i])
					return false;

			return true;
		}

		std::vector<Object *> elements;
	};

	struct TableObject : public Object
	{
		TableObject() {}
		TableObject(const std::unordered_map<Object *, Object *> &elements) : elements(elements) {}
		~TableObject() { std::unordered_map<Object *, Object *>().swap(elements); }

		std::string Stringify() override
		{
			std::string result = "{";
			if (!elements.empty())
			{
				for (auto [key, value] : elements)
					result += key->Stringify() + ":" + value->Stringify() + ",";
				result = result.substr(0, result.size() - 1);
			}
			result += "}";
			return result;
		}
		ObjectType Type() override { return ObjectType::TABLE; }
		void Mark() override
		{
			if (marked)
				return;
			marked = true;

			for (auto [key, value] : elements)
			{
				key->Mark();
				value->Mark();
			}
		}
		void UnMark() override
		{
			if (!marked)
				return;
			marked = false;

			for (auto [key, value] : elements)
			{
				key->UnMark();
				value->UnMark();
			}
		}

		bool IsEqualTo(Object *other) override
		{
			if (!IS_TABLE_OBJ(other))
				return false;

			TableObject *tableOther = TO_TABLE_OBJ(other);

			if (tableOther->elements.size() != elements.size())
				return false;

			for (auto [key1, value1] : elements)
				for (auto [key2, value2] : tableOther->elements)
					if (!key1->IsEqualTo(key2) || !value1->IsEqualTo(value2))
						return false;

			return true;
		}

		std::unordered_map<Object *, Object *> elements;
	};

	struct StructObject : public Object
	{
		StructObject() : context(nullptr) {}
		StructObject(Context *context) : context(context) {}
		~StructObject() {}

		std::string Stringify() override
		{
			std::string result = "struct{\n";
			if (!context->m_Values.empty())
				for (const auto &[key, value] : context->m_Values)
					result += key + "=" + value->Stringify() + "\n";
			result += "}\n";
			return result;
		}
		ObjectType Type() override { return ObjectType::STRUCT; }
		void Mark() override { marked = true; }
		void UnMark() override { marked = false; }
		bool IsEqualTo(Object *other) override
		{
			if (!IS_STRUCT_OBJ(other))
				return false;
			return context->IsEqualTo(TO_STRUCT_OBJ(other)->context);
		}

		Context *context;
	};

	struct RefObject : public Object
	{
		RefObject(std::string_view refObjName) : refObjName(refObjName) {}
		~RefObject() {}

		std::string Stringify() override { return "ref " + refObjName; }
		ObjectType Type() override { return ObjectType::REF; }
		void Mark() override { marked = true; }
		void UnMark() override { marked = false; }
		bool IsEqualTo(Object *other) override
		{
			if (!IS_REF_OBJ(other))
				return false;
			return refObjName == TO_REF_OBJ(other)->refObjName;
		}

		std::string refObjName;
	};
}