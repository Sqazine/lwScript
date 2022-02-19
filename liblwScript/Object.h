#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include "Utils.h"
#include "Context.h"
namespace lws
{
#define TO_INT_OBJ(obj) ((IntNumObject *)obj)
#define TO_REAL_OBJ(obj) ((RealNumObject *)obj)
#define TO_STR_OBJ(obj) ((StrObject *)obj)
#define TO_NIL_OBJ(obj) ((NilObject *)obj)
#define TO_BOOL_OBJ(obj) ((BoolObject *)obj)
#define TO_ARRAY_OBJ(obj) ((ArrayObject *)obj)
#define TO_TABLE_OBJ(obj) ((TableObject *)obj)
#define TO_FUNCTION_OBJ(obj) ((FunctionObject *)obj)
#define TO_FIELD_OBJ(obj) ((FieldObject *)obj)
#define TO_REF_OBJ(obj) ((RefObject *)obj)

#define IS_INT_OBJ(obj) (obj->Type() == ObjectType::INT)
#define IS_REAL_OBJ(obj) (obj->Type() == ObjectType::REAL)
#define IS_STR_OBJ(obj) (obj->Type() == ObjectType::STR)
#define IS_BOOL_OBJ(obj) (obj->Type() == ObjectType::BOOL)
#define IS_NIL_OBJ(obj) (obj->Type() == ObjectType::NIL)
#define IS_ARRAY_OBJ(obj) (obj->Type() == ObjectType::ARRAY)
#define IS_TABLE_OBJ(obj) (obj->Type() == ObjectType::TABLE)
#define IS_FUNCTION_OBJ(obj) (obj->Type() == ObjectType::FUNCTION)
#define IS_FIELD_OBJ(obj) (obj->Type() == ObjectType::FIELD)
#define IS_REF_OBJ(obj) (obj->Type() == ObjectType::REF)

	enum class ObjectType
	{
		INT,
		REAL,
		STR,
		BOOL,
		NIL,
		ARRAY,
		TABLE,
		FUNCTION,
		FIELD,
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

	struct IntNumObject : public Object
	{
		IntNumObject();
		IntNumObject(int64_t value);
		~IntNumObject();

		std::string Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		int64_t value;
	};

	struct RealNumObject : public Object
	{
		RealNumObject();
		RealNumObject(double value);
		~RealNumObject();

		std::string Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

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
		BoolObject();
		BoolObject(bool value);
		~BoolObject();

		std::string Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

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
		ArrayObject();
		ArrayObject(const std::vector<Object *> &elements);
		~ArrayObject();

		std::string Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;

		bool IsEqualTo(Object *other) override;

		std::vector<Object *> elements;
	};

	struct TableObject : public Object
	{
		TableObject();
		TableObject(const std::unordered_map<Object *, Object *> &elements);
		~TableObject();

		std::string Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		std::unordered_map<Object *, Object *> elements;
	};

	struct FunctionObject : public Object
	{
		FunctionObject();
		FunctionObject(int64_t frameIndex);
		~FunctionObject();

		std::string Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;

		bool IsEqualTo(Object *other) override;

		int64_t frameIndex;
	};

	struct RefObject : public Object
	{
		RefObject(std::string_view address);
		~RefObject();

		std::string Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		std::string address;
	};
	struct FieldObject : public Object
	{
		FieldObject();
		FieldObject(std::string_view name,
					const std::unordered_map<std::string, Object *> &members,
					const std::vector<std::pair<std::string, FieldObject *>> &containedFields = {});
		~FieldObject();

		std::string Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		void AssignMemberByName(std::string_view name, Object *value);
		Object *GetMemberByName(std::string_view name);
		Object *GetMemberByAddress(std::string_view address);

		std::string name;
		std::unordered_map<std::string, Object *> members;
		std::vector<std::pair<std::string, FieldObject *>> containedFields;
	};
}