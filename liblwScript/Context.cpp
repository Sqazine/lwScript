#include "Context.h"
#include "Utils.h"
#include "VM.h"
#include "Object.h"
namespace lws
{
	Scope::Scope()
	{
	}
	Scope::~Scope()
	{
	}

	void Scope::DefineVariableByName(std::wstring_view name, ValueDescType objDescType, const Value &value)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
			ASSERT(L"Redefined variable:(" + std::wstring(name) + L") in current context.")
		else
		{
			ValueDesc desc;
			desc.type = objDescType;
			desc.value = value;

			mValues[name.data()] = desc;
		}
	}

	void Scope::DefineVariableByName(std::wstring_view name, const ValueDesc &objectDesc)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
			ASSERT(L"Redefined variable:(" + std::wstring(name) + L") in current context.")
		else
			mValues[name.data()] = objectDesc;
	}

	bool Scope::AssignVariableByName(std::wstring_view name, const Value &value)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
		{
			if (iter->second.type != ValueDescType::CONST)
			{
				mValues[name.data()].value = value;
				return true;
			}
			else
				ASSERT(L"const variable:(" + std::wstring(name) + L") cannot be assigned")
		}
		return false;
	}

	Value Scope::GetVariableByName(std::wstring_view name)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
			return iter->second.value;
		return gInvalidValue;
	}

	bool Scope::AssignVariableByAddress(std::wstring_view address, const Value &value)
	{
		for (auto &[contextKey, contextValue] : mValues)
		{
			if (PointerAddressToString(contextValue.value.object) == address)
			{
				if (contextValue.type != ValueDescType::CONST)
				{
					mValues[contextKey].value = value;
					return true;
				}
				else
					ASSERT(L"const variable at address:(" + std::wstring(address) + L") cannot be assigned")
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
							return true;
						}
				}
				else
					ASSERT(L"const variable at address:(" + std::wstring(address) + L") cannot be assigned")
			}
			else if (IS_TABLE_VALUE(contextValue.value))
			{
				TableObject *table = TO_TABLE_VALUE(contextValue.value);
				if (contextValue.type != ValueDescType::CONST)
				{
					for (auto &[tableKey, tableValue] : table->elements)
						if (PointerAddressToString(tableValue.object) == address)
						{
							table->elements[tableKey] = value;
							return true;
						}
				}
				else
					ASSERT(L"const variable at address:(" + std::wstring(address) + L") cannot be assigned")
			}
			else if (IS_CLASS_VALUE(contextValue.value))
			{
				ClassObject *klass = TO_CLASS_VALUE(contextValue.value);
				if (contextValue.type != ValueDescType::CONST)
				{
					for (auto &[classMemberKey, classMemberValue] : klass->members)
						if (PointerAddressToString(classMemberValue.value.object) == address)
						{
							klass->members[classMemberKey].value = value;
							return true;
						}
				}
				else
					ASSERT(L"const variable at address:(" + std::wstring(address) + L") cannot be assigned")
			}
		}

		return false;
	}

	Value Scope::GetVariableByAddress(std::wstring_view address)
	{
		//first:search the suitable context value in address
		for (auto &[contextKey, contextValue] : mValues)
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
				for (auto &[tableKey, tableValue] : table->elements)
					if (PointerAddressToString(tableValue.object) == address)
						return table->elements[tableKey];
			}
			else if (IS_CLASS_VALUE(contextValue.value))
			{
				ClassObject *klass = TO_CLASS_VALUE(contextValue.value);
				return klass->GetMemberByAddress(address);
			}
		}

		return gInvalidValue;
	}

	void Scope::Mark()
	{
		for (const auto &[k, v] : mValues)
			v.value.Mark();
	}

	void Scope::Reset()
	{
		mValues.clear();
	}

	Context::Context()
	{
		mScopeStack = std::vector<Scope>(SCOPE_STACK_MAX);
		mCurScopeDepth = 0;
	}

	Context::~Context()
	{
	}

	void Context::DefineVariableByName(std::wstring_view name, ValueDescType objDescType, const Value &value)
	{
		CurScope().DefineVariableByName(name, objDescType, value);
	}

	void Context::DefineVariableByName(std::wstring_view name, const ValueDesc &objectDesc)
	{
		CurScope().DefineVariableByName(name, objectDesc);
	}

	void Context::AssignVariableByName(std::wstring_view name, const Value &value)
	{
		if (!CurScope().AssignVariableByName(name, value))
		{
			for (int32_t i = mCurScopeDepth; i >= 0; --i)
				if (mScopeStack[i].AssignVariableByName(name, value))
					return;
			ASSERT(L"Undefine variable:(" + std::wstring(name) + L") in context")
		}
	}

	Value Context::GetVariableByName(std::wstring_view name)
	{
		auto value = CurScope().GetVariableByName(name);
		if (!IS_INVALID_VALUE(value))
			return value;
		else
		{
			for (int32_t i = mCurScopeDepth; i >= 0; --i)
			{
				value = mScopeStack[i].GetVariableByName(name);
				if (!IS_INVALID_VALUE(value))
					return value;
			}
		}
		return gInvalidValue;
	}

	void Context::AssignVariableByAddress(std::wstring_view address, const Value &value)
	{
		if (!CurScope().AssignVariableByAddress(address, value))
		{
			for (int32_t i = mCurScopeDepth; i >= 0; --i)
				if (mScopeStack[i].AssignVariableByAddress(address, value))
					return;
			ASSERT(L"Undefine variable(address:" + std::wstring(address) + L") in current context")
		}
	}

	Value Context::GetVariableByAddress(std::wstring_view address)
	{

		auto value = CurScope().GetVariableByAddress(address);
		if (!IS_INVALID_VALUE(value))
			return value;
		else
		{
			for (int32_t i = mCurScopeDepth; i >= 0; --i)
			{
				value = mScopeStack[i].GetVariableByAddress(address);
				if (!IS_INVALID_VALUE(value))
					return value;
			}
		}
		return gInvalidValue;
	}

	Scope &Context::GetUpScope()
	{
		return mScopeStack[mCurScopeDepth - 1];
	}

	Scope &Context::GetRootScope()
	{
		return mScopeStack[0];
	}

	void Context::EnterScope()
	{
		++mCurScopeDepth;
	}
	void Context::ExitScope()
	{
		if (mCurScopeDepth > 0)
		{
			CurScope().Reset();
			--mCurScopeDepth;
		}
	}

	Scope &Context::CurScope()
	{
		return mScopeStack[mCurScopeDepth];
	}

	void Context::Mark()
	{
		for (int32_t i = 0; i < mCurScopeDepth; ++i)
			mScopeStack[i].Mark();
	}
}