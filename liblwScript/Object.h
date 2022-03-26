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
#define TO_NULL_OBJ(obj) ((NullObject *)obj)
#define TO_BOOL_OBJ(obj) ((BoolObject *)obj)
#define TO_ARRAY_OBJ(obj) ((ArrayObject *)obj)
#define TO_TABLE_OBJ(obj) ((TableObject *)obj)
#define TO_LAMBDA_OBJ(obj) ((LambdaObject *)obj)
#define TO_FIELD_OBJ(obj) ((FieldObject *)obj)
#define TO_REF_OBJ(obj) ((RefObject *)obj)

#define IS_INT_OBJ(obj) (obj->Type() == OBJECT_INT)
#define IS_REAL_OBJ(obj) (obj->Type() == OBJECT_REAL)
#define IS_STR_OBJ(obj) (obj->Type() == OBJECT_STR)
#define IS_BOOL_OBJ(obj) (obj->Type() == OBJECT_BOOL)
#define IS_NULL_OBJ(obj) (obj->Type() == OBJECT_NULL)
#define IS_ARRAY_OBJ(obj) (obj->Type() == OBJECT_ARRAY)
#define IS_TABLE_OBJ(obj) (obj->Type() == OBJECT_TABLE)
#define IS_LAMBDA_OBJ(obj) (obj->Type() == OBJECT_LAMBDA)
#define IS_FIELD_OBJ(obj) (obj->Type() == OBJECT_FIELD)
#define IS_REF_OBJ(obj) (obj->Type() == OBJECT_REF)

	enum ObjectType
	{
		OBJECT_INT,
		OBJECT_REAL,
		OBJECT_STR,
		OBJECT_BOOL,
		OBJECT_NULL,
		OBJECT_ARRAY,
		OBJECT_TABLE,
		OBJECT_LAMBDA,
		OBJECT_FIELD,
		OBJECT_REF
	};

	struct Object
	{
		Object() : marked(false), next(nullptr) {}
		virtual ~Object() {}

		virtual std::wstring Stringify() = 0;
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

		std::wstring Stringify() override;
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

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		double value;
	};

	struct StrObject : public Object
	{
		StrObject() {}
		StrObject(std::wstring_view value) : value(value) {}
		~StrObject() {}

		std::wstring Stringify() override { return value; }
		ObjectType Type() override { return OBJECT_STR; }
		void Mark() override { marked = true; }
		void UnMark() override { marked = false; }
		bool IsEqualTo(Object *other) override
		{
			if (!IS_STR_OBJ(other))
				return false;
			return value == TO_STR_OBJ(other)->value;
		}

		std::wstring value;
	};

	struct BoolObject : public Object
	{
		BoolObject();
		BoolObject(bool value);
		~BoolObject();

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		bool value;
	};

	struct NullObject : public Object
	{
		NullObject();
		~NullObject();

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;
	};

	struct ArrayObject : public Object
	{
		ArrayObject();
		ArrayObject(const std::vector<Object *> &elements);
		~ArrayObject();

		std::wstring Stringify() override;
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

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		std::unordered_map<Object *, Object *> elements;
	};

	struct LambdaObject : public Object
	{
		LambdaObject();
		LambdaObject(int64_t frameIndex);
		~LambdaObject();

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;

		bool IsEqualTo(Object *other) override;

		int64_t frameIndex;
	};

	struct RefObject : public Object
	{
		RefObject(std::wstring_view name, Object *index = nullptr);
		~RefObject();

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		std::wstring name;
		Object *index;
	};
	struct FieldObject : public Object
	{
		FieldObject();
		FieldObject(std::wstring_view name,
					const std::unordered_map<std::wstring, Object *> &members,
					const std::vector<std::pair<std::wstring, FieldObject *>> &containedFields = {});
		~FieldObject();

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		void AssignMemberByName(std::wstring_view name, Object *value);
		Object *GetMemberByName(std::wstring_view name);

		std::wstring name;
		std::unordered_map<std::wstring, Object *> members;
		std::vector<std::pair<std::wstring, FieldObject *>> containedFields;
	};
}