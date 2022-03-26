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

	Context *Context::GetUpContext()
	{
		return mUpContext;
	}

	void Context::SetUpContext(Context *env)
	{
		mUpContext = env;
	}
}