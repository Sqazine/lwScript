#pragma once
#include <string>
#include <unordered_map>
#include "Config.h"
#include "Utils.h"
namespace lwscript
{
	enum ValueType
	{
		VALUE_NULL,
		VALUE_INT,
		VALUE_REAL,
		VALUE_BOOL,
		VALUE_OBJECT,
	};

	struct LWSCRIPT_API Value
	{
		Value();
		Value(int64_t integer);
		Value(double number);
		Value(bool boolean);
		Value(struct Object *object);
		~Value();

		std::wstring ToString() const;
		void Mark(class Allocator *allocator) const;
		void UnMark() const;
		Value Clone() const;

		ValueType type;
		Privilege privilege = Privilege::MUTABLE;

		union
		{
			int64_t integer;
			double realnum;
			bool boolean;
			struct Object *object;
		};
	};

	LWSCRIPT_API bool operator==(const Value &left, const Value &right);
	LWSCRIPT_API bool operator!=(const Value &left, const Value &right);

	struct LWSCRIPT_API ValueHash
	{
		size_t operator()(const Value &v) const;
	};

	struct LWSCRIPT_API ValueVecHash
	{
		size_t operator()(const std::vector<Value> &vec) const;
	};

	using ValueUnorderedMap = std::unordered_map<Value, Value, ValueHash>;
	using ValueVecUnorderedMap = std::unordered_map<std::vector<Value>, std::vector<Value>, ValueVecHash>;
}