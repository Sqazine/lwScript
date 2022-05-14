#include "Context.h"
#include "Utils.h"
#include "VM.h"
#include "Object.h"
namespace lws
{
	Context::Context()
		: mUpContext(nullptr)
	{
	}
	Context::Context(Context *upContext)
		: mUpContext(upContext)
	{
	}
	Context::~Context()
	{
	}

	void Context::DefineVariableByName(std::wstring_view name, ValueDescType objDescType, const Value &value)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
			Assert(L"Redefined variable:(" + std::wstring(name) + L") in current context.");
		else
		{
			ValueDesc desc;
			desc.type = objDescType;
			desc.value = value;

			mValues[name.data()] = desc;
		}
	}

	void Context::DefineVariableByName(std::wstring_view name, const ValueDesc &objectDesc)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
			Assert(L"Redefined variable:(" + std::wstring(name) + L") in current context.");
		else
			mValues[name.data()] = objectDesc;
	}

	void Context::AssignVariableByName(std::wstring_view name, const Value &value)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
		{
			if (iter->second.type != ValueDescType::CONST)
				mValues[name.data()].value = value;
			else
				Assert(L"const variable:(" + std::wstring(name) + L") cannot be assigned");
		}
		else if (mUpContext != nullptr)
			mUpContext->AssignVariableByName(name, value);
		else
			Assert(L"Undefine variable:(" + std::wstring(name) + L") in current context");
	}

	Value Context::GetVariableByName(std::wstring_view name)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
			return iter->second.value;
		if (mUpContext != nullptr)
			return mUpContext->GetVariableByName(name);
		return gInvalidValue;
	}

	void Context::AssignVariableByAddress(std::wstring_view address, const Value &value)
	{
		for (auto& [contextKey, contextValue] : mValues)
		{
			if (PointerAddressToString(contextValue.value.object) == address)
			{
				if (contextValue.type != ValueDescType::CONST)
				{
					mValues[contextKey].value = value;
					return;
				}
				else
					Assert(L"const variable at address:(" + std::wstring(address) + L") cannot be assigned");
			}
			else if (IS_ARRAY_VALUE(contextValue.value))
			{
				ArrayObject *array = TO_ARRAY_VALUE(contextValue.value);
				if (contextValue.type != ValueDescType::CONST)
				{
					for (size_t i = 0; i < array->elements.size(); ++i)
						if (PointerAddressToString(array->elements[i].object) == address)
						{
							array->elements[i] = value;
							return;
						}
				}
				else
					Assert(L"const variable at address:(" + std::wstring(address) + L") cannot be assigned");
			}
			else if (IS_TABLE_VALUE(contextValue.value))
			{
				TableObject *table = TO_TABLE_VALUE(contextValue.value);
				if (contextValue.type != ValueDescType::CONST)
				{
					for (auto& [tableKey, tableValue] : table->elements)
						if (PointerAddressToString(tableValue.object) == address)
						{
							table->elements[tableKey] = value;
							return;
						}
				}
				else
					Assert(L"const variable at address:(" + std::wstring(address) + L") cannot be assigned");
			}
			else if (IS_FIELD_VALUE(contextValue.value))
			{
				FieldObject *field = TO_FIELD_VALUE(contextValue.value);
				if (contextValue.type != ValueDescType::CONST)
				{
					for (auto& [classMemberKey, classMemberValue] : field->members)
						if (PointerAddressToString(classMemberValue.value.object) == address)
						{
							field->members[classMemberKey].value = value;
							return;
						}
				}
				else
					Assert(L"const variable at address:(" + std::wstring(address) + L") cannot be assigned");
			}
		}

		if (mUpContext)
			mUpContext->AssignVariableByAddress(address, value);
		else
			Assert(L"Undefine variable(address:" + std::wstring(address) + L") in current context");
	}

	Value Context::GetVariableByAddress(std::wstring_view address)
	{
		//first:search the suitable context value in address
		for (auto& [contextKey, contextValue] : mValues)
			if (PointerAddressToString(contextValue.value.object) == address)
				return contextValue.value;

		//second:search the address in the specific object value
		for (auto [contextKey, contextValue] : mValues)
		{
			if (IS_ARRAY_VALUE(contextValue.value))
			{
				ArrayObject *array = TO_ARRAY_VALUE(contextValue.value);
				for (size_t i = 0; i < array->elements.size(); ++i)
					if (PointerAddressToString(array->elements[i].object) == address)
						return array->elements[i];
			}
			else if (IS_TABLE_VALUE(contextValue.value))
			{
				TableObject *table = TO_TABLE_VALUE(contextValue.value);
				for (auto& [tableKey, tableValue] : table->elements)
					if (PointerAddressToString(tableValue.object) == address)
						return table->elements[tableKey];
			}
			else if (IS_FIELD_VALUE(contextValue.value))
			{
				FieldObject *field = TO_FIELD_VALUE(contextValue.value);
				return field->GetMemberByAddress(address);
			}
		}

		if (mUpContext)
			return mUpContext->GetVariableByAddress(address);

		return gInvalidValue;
	}

	Context *Context::GetUpContext()
	{
		return mUpContext;
	}

	void Context::SetUpContext(Context *env)
	{
		mUpContext = env;
	}

	Context *Context::GetRoot()
	{
		Context *ptr = this;
		while (ptr->mUpContext != nullptr)
			ptr = ptr->mUpContext;
		return ptr;
	}
}