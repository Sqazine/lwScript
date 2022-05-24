#include "Library.h"
#include "VM.h"
#include <stdio.h>
#include <ctime>
namespace lws
{

	Library::Library(VM* vm)
		: mVMHandle(vm)
	{
	}
	Library::~Library()
	{
		std::unordered_map<std::wstring, std::function<Value(const std::vector<Value>&)>>().swap(mNativeFunctions);
	}

	void Library::AddNativeFunction(std::wstring_view name, std::function<Value(const std::vector<Value>&)> fn)
	{
		auto iter = mNativeFunctions.find(name.data());
		if (iter != mNativeFunctions.end())
			ASSERT(std::wstring(L"Already exists native function:") + name.data())
		mNativeFunctions[name.data()] = fn;
	}
	std::function<Value(const std::vector<Value>&)> Library::GetNativeFunction(std::wstring_view fnName)
	{
		auto iter = mNativeFunctions.find(fnName.data());
		if (iter != mNativeFunctions.end())
			return iter->second;
		ASSERT(std::wstring(L"No native function:") + fnName.data())

		return nullptr;
	}
	bool Library::HasNativeFunction(std::wstring_view name)
	{
		auto iter = mNativeFunctions.find(name.data());
		if (iter != mNativeFunctions.end())
			return true;
		return false;
	}

	IO::IO(VM* vm)
		: Library(vm)
	{
		mNativeFunctions[L"print"] = [this](const std::vector<Value>& args) -> Value
		{
			if (args.empty())
				return gInvalidValue;

			if (args.size() == 1)
			{
				std::wcout << args[0].Stringify();
				return gInvalidValue;
			}

			if (!IS_STR_VALUE(args[0]))
			{
				for (const auto& arg : args)
					std::wcout << arg.Stringify();
				return gInvalidValue;
			}

			std::wstring content = TO_STR_VALUE(args[0])->value;

			if (args.size() != 1) //formatting output
			{
				size_t pos = content.find(L"{}");
				size_t argpos = 1;
				while (pos != std::wstring::npos)
				{
					if (argpos < args.size())
						content.replace(pos, 2, args[argpos++].Stringify());
					else
						content.replace(pos, 2, L"null");
					pos = content.find(L"{}");
				}
			}

			size_t pos = content.find(L"\\n");
			while (pos != std::wstring::npos)
			{
				content[pos] = '\n';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\n");
			}

			pos = content.find(L"\\t");
			while (pos != std::wstring::npos)
			{
				content[pos] = '\t';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\t");
			}

			pos = content.find(L"\\r");
			while (pos != std::wstring::npos)
			{
				content[pos] = '\r';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\r");
			}

			std::wcout << content;
			return gInvalidValue;
		};

		mNativeFunctions[L"println"] = [this](const std::vector<Value>& args) -> Value
		{
			if (args.empty())
				return gInvalidValue;

			if (args.size() == 1)
			{
				std::wcout << args[0].Stringify() << std::endl;
				return gInvalidValue;
			}

			if (!IS_STR_VALUE(args[0]))
			{
				for (const auto& arg : args)
					std::wcout << arg.Stringify() << std::endl;
				return gInvalidValue;
			}

			std::wstring content = TO_STR_VALUE(args[0])->value;

			if (args.size() != 1) //formatting output
			{
				size_t pos = (int32_t)content.find(L"{}");
				size_t argpos = 1;
				while (pos != std::wstring::npos)
				{
					if (argpos < args.size())
						content.replace(pos, 2, args[argpos++].Stringify());
					else
						content.replace(pos, 2, L"null");
					pos = content.find(L"{}");
				}
			}

			size_t pos = content.find(L"\\n");
			while (pos != std::wstring::npos)
			{
				content[pos] = '\n';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\n");
			}

			pos = content.find(L"\\t");
			while (pos != std::wstring::npos)
			{
				content[pos] = '\t';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\t");
			}

			pos = content.find(L"\\r");
			while (pos != std::wstring::npos)
			{
				content[pos] = L'\r';
				content.replace(pos + 1, 1, L""); //erase a char
				pos = content.find(L"\\r");
			}

			std::wcout << content << std::endl;
			return gInvalidValue;
		};
	}

	DataStructure::DataStructure(VM* vm)
		: Library(vm)
	{
		mNativeFunctions[L"sizeof"] = [this](const std::vector<Value>& args) -> Value
		{
			if (args.empty() || args.size() > 1)
				ASSERT(L"[Native function 'sizeof']:Expect a argument.")

			if (IS_ARRAY_VALUE(args[0]))
				return Value((int64_t)TO_ARRAY_VALUE(args[0])->elements.size());
			else if (IS_TABLE_VALUE(args[0]))
				return Value((int64_t)TO_TABLE_VALUE(args[0])->elements.size());
			else if (IS_STR_VALUE(args[0]))
				return Value((int64_t)TO_STR_VALUE(args[0])->value.size());
			else
				ASSERT(L"[Native function 'sizeof']:Expect a array,table ot string argument.")
			return gInvalidValue;
		};

		mNativeFunctions[L"insert"] = [this](std::vector<Value> args) -> Value
		{
			if (args.empty() || args.size() != 3)
				ASSERT(L"[Native function 'insert']:Expect 3 arguments,the arg0 must be array,table or string object.The arg1 is the index object.The arg2 is the value object.")

			if (IS_ARRAY_VALUE(args[0]))
			{
				ArrayObject* array = TO_ARRAY_VALUE(args[0]);
				if (!IS_INT_VALUE(args[1]))
					ASSERT(L"[Native function 'insert']:Arg1 must be integer type while insert to a array")

				int64_t iIndex = TO_INT_VALUE(args[1]);

				if (iIndex < 0 || iIndex >= (int64_t)array->elements.size())
					ASSERT(L"[Native function 'insert']:Index out of array's range")

				array->elements.insert(array->elements.begin() + iIndex, 1, args[2]);
			}
			else if (IS_TABLE_VALUE(args[0]))
			{
				TableObject* table = TO_TABLE_VALUE(args[0]);

				for (auto [key, value] : table->elements)
					if (key==args[1])
						ASSERT(L"[Native function 'insert']:Already exist value in the table object of arg1" + args[1].Stringify())

				table->elements[args[1]] = args[2];
			}
			else if (IS_STR_VALUE(args[0]))
			{
				auto string = TO_STR_VALUE(args[0])->value;
				if (!IS_INT_VALUE(args[1]))
					ASSERT(L"[Native function 'insert']:Arg1 must be integer type while insert to a array");

				int64_t iIndex = TO_INT_VALUE(args[1]);

				if (iIndex < 0 || iIndex >= (int64_t)string.size())
					ASSERT(L"[Native function 'insert']:Index out of array's range");

				string.insert(iIndex, args[2].Stringify());
			}
			else
				ASSERT(L"[Native function 'insert']:Expect a array,table ot string argument.")
			return gInvalidValue;
		};

		mNativeFunctions[L"erase"] = [this](const std::vector<Value>& args) -> Value
		{
			if (args.empty() || args.size() != 2)
				ASSERT(L"[Native function 'erase']:Expect 2 arguments,the arg0 must be array,table or string object.The arg1 is the corresponding index object.")

			if (IS_ARRAY_VALUE(args[0]))
			{
				ArrayObject* array = TO_ARRAY_VALUE(args[0]);
				if (!IS_INT_VALUE(args[1]))
					ASSERT(L"[Native function 'erase']:Arg1 must be integer type while insert to a array")

				int64_t iIndex = TO_INT_VALUE(args[1]);

				if (iIndex < 0 || iIndex >= (int64_t)array->elements.size())
					ASSERT(L"[Native function 'erase']:Index out of array's range")

				array->elements.erase(array->elements.begin() + iIndex);
			}
			else if (IS_TABLE_VALUE(args[0]))
			{
				TableObject* table = TO_TABLE_VALUE(args[0]);

				bool hasValue = false;

				for (auto it = table->elements.begin(); it != table->elements.end(); ++it)
					if (it->first==args[1])
					{
						table->elements.erase(it);
						hasValue = true;
						break;
					}

				if (!hasValue)
					ASSERT(L"[Native function 'erase']:No corresponding index in table.")
			}
			else if (IS_STR_VALUE(args[0]))
			{
				auto string = TO_STR_VALUE(args[0])->value;
				if (!IS_INT_VALUE(args[1]))
					ASSERT(L"[Native function 'erase']:Arg1 must be integer type while insert to a array")

				int64_t iIndex = TO_INT_VALUE(args[1]);

				if (iIndex < 0 || iIndex >= (int64_t)string.size())
					ASSERT(L"[Native function 'erase']:Index out of array's range")

				string.erase(string.begin() + iIndex);
			}
			else
				ASSERT(L"[Native function 'erase']:Expect a array,table ot string argument.")
			return gInvalidValue;
		};
	}

	Memory::Memory(VM *vm)
		: Library(vm)
	{
		mNativeFunctions[L"addressof"] = [this](const std::vector<Value>& args) -> Value
		{
			if (args.empty() || args.size() != 1)
				ASSERT(L"[Native function 'addressof']:Expect 1 arguments.")

			if(!IS_OBJECT_VALUE(args[0]))
				ASSERT(L"[Native function 'addressof']:The arg0 is a value,only object has address.")

			return mVMHandle->CreateStrObject(PointerAddressToString(args[0].object));
		};
	}

	Time::Time(class VM *vm)
	:Library(vm)
	{
		mNativeFunctions[L"clock"]=[this](const std::vector<Value>& args)->Value
		{
			return Value((double)clock() / CLOCKS_PER_SEC);
		};
	}

	std::unordered_map<std::wstring, Library*> LibraryManager::mLibraries;

	void LibraryManager::RegisterLibrary(std::wstring_view name, Library* lib)
	{
		auto iter = mLibraries.find(name.data());
		if (iter != mLibraries.end())
			ASSERT(L"Already exists a native function library:" + std::wstring(name))
		mLibraries[name.data()] = lib;
	}
	bool LibraryManager::HasNativeFunction(std::wstring_view name)
	{
		for (const auto& lib : mLibraries)
			if (lib.second->HasNativeFunction(name))
				return true;
		return false;
	}
}