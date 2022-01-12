#include "Context.h"
#include "Utils.h"
#include "VM.h"
#include "Object.h"
namespace lws
{
	Context::Context() : m_UpContext(nullptr) {}
	Context::Context(Context* upContext) : m_UpContext(upContext) {}
	Context::~Context() {}

	void Context::DefineVariableByName(std::string_view name, Object* value)
	{
		auto iter = m_Values.find(name.data());
		if (iter != m_Values.end())
			Assert("Redefined variable:" + std::string(name) + " in current context.");
		else
			m_Values[name.data()] = value;
	}

	void Context::AssignVariableByName(std::string_view name, Object* value)
	{
		auto iter = m_Values.find(name.data());
		if (iter != m_Values.end())
			m_Values[name.data()] = value;
		else if (m_UpContext != nullptr)
			m_UpContext->AssignVariableByName(name, value);
		else
			Assert("Undefine variable:" + std::string(name) + " in current context");
	}

	Object* Context::GetVariableByName(std::string_view name)
	{
		auto iter = m_Values.find(name.data());
		if (iter != m_Values.end())
			return iter->second;
		if (m_UpContext != nullptr)
			return m_UpContext->GetVariableByName(name);
		return nullptr;
	}

	void Context::AssignVariableByAddress(std::string_view address, Object* value)
	{
		for (auto [contextKey, contextValue] : m_Values)
		{
			if (PointerAddressToString(contextValue) == address)
			{
				m_Values[contextKey] = value;
				return;
			}
			else if (IS_ARRAY_OBJ(contextValue))
			{
				ArrayObject* array = TO_ARRAY_OBJ(contextValue);
				for (size_t i = 0; i < array->elements.size(); ++i)
					if (PointerAddressToString(array->elements[i]) == address)
					{
						array->elements[i] = value;
						return;
					}
			}
			else if (IS_TABLE_OBJ(contextValue))
			{
				TableObject* table = TO_TABLE_OBJ(contextValue);
				for (auto [tableKey, tableValue] : table->elements)
					if (PointerAddressToString(tableValue) == address)
					{
						table->elements[tableKey] = value;
						return;
					}
			}
			else if (IS_FIELD_OBJ(contextValue))
			{
				FieldObject* field = TO_FIELD_OBJ(contextValue);
				for (auto [classMemberKey, classMemberValue] : field->members)
					if (PointerAddressToString(classMemberValue) == address)
					{
						field->members[classMemberKey] = value;
						return;
					}
			}
		}

		if (m_UpContext)
			m_UpContext->AssignVariableByAddress(address, value);
		else
			Assert("Undefine variable(address:" + std::string(address) + ") in current context");
	}

	Object* Context::GetVariableByAddress(std::string_view address)
	{
		for (auto [contextKey, contextValue] : m_Values)
		{
			if (PointerAddressToString(contextValue) == address)
				return contextValue;
			else if (IS_ARRAY_OBJ(contextValue))
			{
				ArrayObject* array = TO_ARRAY_OBJ(contextValue);
				for (size_t i = 0; i < array->elements.size(); ++i)
					if (PointerAddressToString(array->elements[i]) == address)
						return array->elements[i];
			}
			else if (IS_TABLE_OBJ(contextValue))
			{
				TableObject* table = TO_TABLE_OBJ(contextValue);
				for (auto [tableKey, tableValue] : table->elements)
					if (PointerAddressToString(tableValue) == address)
						return table->elements[tableKey];
			}
			else if (IS_FIELD_OBJ(contextValue))
			{
				FieldObject* field = TO_FIELD_OBJ(contextValue);
				for (auto [classMemberKey, classMemberValue] : field->members)
					if (PointerAddressToString(classMemberValue) == address)
						return field->members[classMemberKey];
			}
		}

		if (m_UpContext)
			return m_UpContext->GetVariableByAddress(address);

		return nullptr;
	}

	Context* Context::GetUpContext()
	{
		return m_UpContext;
	}

	void Context::SetUpContext(Context* env)
	{
		m_UpContext = env;
	}

	const std::unordered_map<std::string, struct Object*>& Context::GetValues() const
	{
		return m_Values;
	}
}