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
#define TO_STR_OBJ(obj) ((StrObject *)obj)
#define TO_ARRAY_OBJ(obj) ((ArrayObject *)obj)
#define TO_TABLE_OBJ(obj) ((TableObject *)obj)
#define TO_LAMBDA_OBJ(obj) ((LambdaObject *)obj)
#define TO_CLASS_OBJ(obj) ((ClassObject *)obj)
#define TO_REF_OBJ(obj) ((RefObject *)obj)

#define IS_STR_OBJ(obj) (obj->Type() == OBJECT_STR)
#define IS_ARRAY_OBJ(obj) (obj->Type() == OBJECT_ARRAY)
#define IS_TABLE_OBJ(obj) (obj->Type() == OBJECT_TABLE)
#define IS_LAMBDA_OBJ(obj) (obj->Type() == OBJECT_LAMBDA)
#define IS_CLASS_OBJ(obj) (obj->Type() == OBJECT_CLASS)
#define IS_REF_OBJ(obj) (obj->Type() == OBJECT_REF)

	enum ObjectType
	{
		OBJECT_STR,
		OBJECT_ARRAY,
		OBJECT_TABLE,
		OBJECT_LAMBDA,
		OBJECT_CLASS,
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

	struct StrObject : public Object
	{
		StrObject();
		StrObject(std::wstring_view value);
		~StrObject();

		std::wstring Stringify() override;
		ObjectType Type() override;
		void Mark() override;
		void UnMark() override;
		bool IsEqualTo(Object *other) override;
		std::wstring value;
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

	struct ClassObject : public Object
	{
		ClassObject();
		ClassObject(std::wstring_view name,
					const std::unordered_map<std::wstring, ValueDesc> &members,
					const std::vector<std::pair<std::wstring, ClassObject *>> &parentClasses = {});
		~ClassObject();

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
		std::vector<std::pair<std::wstring, ClassObject *>> parentClasses;
	};
}