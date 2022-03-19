#include "Context.h"
#include "Utils.h"
#include "VM.h"
#include "Object.h"
namespace lws
{
	Context::Context() : mUpContext(nullptr) {}
	Context::Context(Context *upContext) : mUpContext(upContext) {}
	Context::~Context() {}

	void Context::DefineVariableByName(std::string_view name, ObjectDescType objDescType, Object *value)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
			Assert("Redefined variable:" + std::string(name) + " in current context.");
		else
		{
			ObjectDesc desc;
			desc.type = objDescType;
			desc.object = value;

			mValues[name.data()] = desc;
		}
	}

	void Context::DefineVariableByName(std::string_view name, const ObjectDesc &objectDesc)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
			Assert("Redefined variable:" + std::string(name) + " in current context.");
		else
			mValues[name.data()] = objectDesc;
	}

	void Context::AssignVariableByName(std::string_view name, Object *value)
	{
		auto iter = mValues.find(name.data());
		if (iter != mValues.end())
		{
			if(iter->second.type!=ObjectDescType::CONST)
			mValues[name.data()].object=value;
		}
		else if (mUpContext != nullptr)
			mUpContext->AssignVariableByName(name, value);
		else
			Assert("Undefine variable:" + std::string(name) + " in current context");
	}

	Object *Context::GetVariableByName(std::string_view name)
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