#pragma once
#include <string>
#include <unordered_map>
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
		constexpr Value() noexcept;
		constexpr Value(int64_t integer) noexcept;
		constexpr Value(double number) noexcept;
		constexpr Value(bool boolean) noexcept;
		constexpr Value(struct Object *object) noexcept; 
		constexpr ~Value() noexcept = default;

		STD_STRING ToString() const;
		void Mark() const;
		void UnMark() const;

		std::vector<uint8_t> Serialize() const;
		void Deserialize(const std::vector<uint8_t> &data);

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
		size_t operator()(const Value *v) const;
		size_t operator()(const Value &v) const;
	};

	using ValueUnorderedMap = std::unordered_map<Value, Value, ValueHash>;

	size_t HashValueList(Value *start, size_t count);
	size_t HashValueList(Value *start, Value *end);
}