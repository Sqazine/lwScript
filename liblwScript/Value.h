#pragma once
#include <string>
#include <unordered_map>
#include "Config.h"
#include "Utils.h"
namespace lwscript
{
	enum ValueKind : uint8_t
	{
		NIL,
		INT,
		REAL,
		BOOL,
		OBJECT,
	};

	struct LWSCRIPT_API Value
	{
		Value();
		Value(int64_t integer);
		Value(double number);
		Value(bool boolean);
		Value(struct Object *object);
		~Value() = default;

		STD_STRING ToString() const;
		void Mark() const;
		void UnMark() const;
		Value Clone() const;

		ValueKind kind;
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