#include "Context.h"
#include "Utils.h"
#include "VM.h"
#include "Object.h"
namespace lws
{
	Context::Context() : mUpContext(nullptr) {}
	Context::Context(Context *upContext) : mUpContext(upContext) {}
	Context::~Context() {}

	void Context::DefineVariableByName(std::wstring_view name, ObjectDescType objDescType, Object *value)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
			Assert(L"Redefined variable:(" + std::wstring(name) + L") in current context.");
		else
		{
			ObjectDesc desc;
			desc.type = objDescType;
			desc.object = value;

			mValues[name.data()] = desc;
		}
	}

	void Context::DefineVariableByName(std::wstring_view name, const ObjectDesc &objectDesc)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
			Assert(L"Redefined variable:(" + std::wstring(name) + L") in current context.");
		else
			mValues[name.data()] = objectDesc;
	}

	void Context::AssignVariableByName(std::wstring_view name, Object *value)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
		{
			if (iter->second.type != ObjectDescType::CONST)
				mValues[name.data()].object = value;
			else
				Assert(L"const variable:(" + std::wstring(name) + L") cannot be assigned");
		}
		else if (mUpContext != nullptr)
			mUpContext->AssignVariableByName(name, value);
		else
			Assert(L"Undefine variable:(" + std::wstring(name) + L") in current context");
	}

	Object *Context::GetVariableByName(std::wstring_view name)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
			return iter->second.object;
		if (mUpContext != nullptr)
			return mUpContext->GetVariableByName(name);
		return nullptr;
	}

	void Context::AssignVariableByAddress(std::wstring_view address, Object *value)
	{
		for (auto [contextKey, contextValue] : mValues)
		{
			if (PointerAddressToString(contextValue.object) == address)
			{
				if (contextValue.type != ObjectDescType::CONST)
				{
					mValues[contextKey].object = value;
					return;
				}
				else
					Assert(L"const variable at address:(" + std::wstring(address) + L") cannot be assigned");
			}
			else if (IS_ARRAY_OBJ(contextValue.object))
			{
				ArrayObject *array = TO_ARRAY_OBJ(contextValue.object);
				if (contextValue.type != ObjectDescType::CONST)
				{
					for (size_t i = 0; i < array->elements.size(); ++i)
						if (PointerAddressToString(array->elements[i]) == address)
						{
							array->elements[i] = value;
							return;
						}
				}
				else
					Assert(L"const variable at address:(" + std::wstring(address) + L") cannot be assigned");
			}
			else if (IS_TABLE_OBJ(contextValue.object))
			{
				TableObject *table = TO_TABLE_OBJ(contextValue.object);
				if (contextValue.type != ObjectDescType::CONST)
				{
					for (auto [tableKey, tableValue] : table->elements)
						if (PointerAddressToString(tableValue) == address)
						{
							table->elements[tableKey] = value;
							return;
						}
				}
				else
					Assert(L"const variable at address:(" + std::wstring(address) + L") cannot be assigned");
			}
			else if (IS_FIELD_OBJ(contextValue.object))
			{
				FieldObject *field = TO_FIELD_OBJ(contextValue.object);
				if (contextValue.type != ObjectDescType::CONST)
				{
					for (auto [classMemberKey, classMemberValue] : field->members)
						if (PointerAddressToString(classMemberValue.object) == address)
						{
							field->members[classMemberKey].object = value;
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

	Object *Context::GetVariableByAddress(std::wstring_view address)
	{
		//first:search the suitable context value in address
		for (auto [contextKey, contextValue] : mValues)
			if (PointerAddressToString(contextValue.object) == address)
				return contextValue.object;

		//second:search the address in the specific object value
		for (auto [contextKey, contextValue] : mValues)
		{
			if (IS_ARRAY_OBJ(contextValue.object))
			{
				ArrayObject *array = TO_ARRAY_OBJ(contextValue.object);
				for (size_t i = 0; i < array->elements.size(); ++i)
					if (PointerAddressToString(array->elements[i]) == address)
						return array->elements[i];
			}
			else if (IS_TABLE_OBJ(contextValue.object))
			{
				TableObject *table = TO_TABLE_OBJ(contextValue.object);
				for (auto [tableKey, tableValue] : table->elements)
					if (PointerAddressToString(tableValue) == address)
						return table->elements[tableKey];
			}
			else if (IS_FIELD_OBJ(contextValue.object))
			{
				FieldObject *field = TO_FIELD_OBJ(contextValue.object);
				return field->GetMemberByAddress(address);
			}
		}

		if (mUpContext)
			return mUpContext->GetVariableByAddress(address);

		return nullptr;
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