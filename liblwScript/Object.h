#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include "Utils.h"
#include "Value.h"
#include "Context.h"
namespace lws
{

#define TO_ARRAY_OBJ(obj) ((ArrayObject *)obj)
#define TO_TABLE_OBJ(obj) ((TableObject *)obj)
#define TO_LAMBDA_OBJ(obj) ((LambdaObject *)obj)
#define TO_FIELD_OBJ(obj) ((FieldObject *)obj)
#define TO_REF_OBJ(obj) ((RefObject *)obj)

#define IS_ARRAY_OBJ(obj) (obj->Type() == OBJECT_ARRAY)
#define IS_TABLE_OBJ(obj) (obj->Type() == OBJECT_TABLE)
#define IS_LAMBDA_OBJ(obj) (obj->Type() == OBJECT_LAMBDA)
#define IS_FIELD_OBJ(obj) (obj->Type() == OBJECT_FIELD)
#define IS_REF_OBJ(obj) (obj->Type() == OBJECT_REF)

	enum ObjectType
	{
		OBJECT_ARRAY,
		OBJECT_TABLE,
		OBJECT_LAMBDA,
		OBJECT_FIELD,
		OBJECT_REF,
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
	struct ArrayObject : public Object
	{
		ArrayObject();
		ArrayObject(const std::vector<Value> &elements);
		~ArrayObject();

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;

		bool IsEqualTo(Object *other) override;

		std::vector<Value> elements;
	};

	struct TableObject : public Object
	{
		TableObject();
		TableObject(const ValueUnorderedMap &elements);
		~TableObject();

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		ValueUnorderedMap elements;
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
		RefObject(std::wstring_view name, const Value &index);
		RefObject(std::wstring_view address);
		~RefObject();

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		bool isAddressReference;
		union
		{
			struct
			{
				std::wstring name;
				Value index;
			};
			std::wstring address;
		};
	};

	struct FieldObject : public Object
	{
		FieldObject();
		FieldObject(std::wstring_view name,
					const std::unordered_map<std::wstring, ValueDesc> &members,
					const std::vector<std::pair<std::wstring, FieldObject *>> &containedFields = {});
		~FieldObject();

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;

		void AssignMemberByName(std::wstring_view name, const Value &value);
		Value GetMemberByName(std::wstring_view name);
		Value GetMemberByAddress(std::wstring_view address);

		std::wstring name;
		std::unordered_map<std::wstring, ValueDesc> members;
		std::vector<std::pair<std::wstring, FieldObject *>> containedFields;
	};
}