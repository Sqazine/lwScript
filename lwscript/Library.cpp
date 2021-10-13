#include "Library.h"
#include "VM.h"
#include <stdio.h>
namespace lws
{

    Library::Library(VM *vm)
        : m_VMHandle(vm)
    {
    }
    Library::~Library()
    {
        std::unordered_map<std::string, std::function<Object *(std::vector<Object *>)>>().swap(m_NativeFunctions);
    }

    void Library::AddNativeFunction(std::string_view name, std::function<Object *(std::vector<Object *>)> fn)
    {
        auto iter = m_NativeFunctions.find(name.data());
        if (iter != m_NativeFunctions.end())
            Assert(std::string("Already exists native function:") + name.data());
        m_NativeFunctions[name.data()] = fn;
    }
    std::function<Object *(std::vector<Object *>)> Library::GetNativeFunction(std::string_view fnName)
    {
        auto iter = m_NativeFunctions.find(fnName.data());
        if (iter != m_NativeFunctions.end())
            return iter->second;
        Assert(std::string("No native function:") + fnName.data());

        return nullptr;
    }
    bool Library::HasNativeFunction(std::string_view name)
    {
        auto iter = m_NativeFunctions.find(name.data());
        if (iter != m_NativeFunctions.end())
            return true;
        return false;
    }

    IO::IO(VM *vm)
        : Library(vm)
    {
		m_NativeFunctions["print"] = [this](std::vector<Object*> args) -> Object*
		{
			if (args.empty())
				return nullptr;

			if (args[0]->Type() != ObjectType::STR)
				Assert("[native function 'println']:Invalid argument:The first argument must be string type.");


			std::string content = TO_STR_OBJ(args[0])->value;

			if (args.size() != 1)//formatting output
            {
				int32_t pos = (int32_t)content.find("{}");
				int32_t argpos = 1;
				while (pos != std::string::npos)
				{
					if (argpos < args.size())
						content.replace(pos, 2, args[argpos++]->Stringify());
					else
						content.replace(pos, 2, "nil");
					pos = content.find("{}");
				}
			}

			int32_t pos = (int32_t)content.find("\\n");
			while (pos != std::string::npos)
			{
				content[pos]='\n';
                content.replace(pos+1, 1, "");//erase a char
				pos = content.find("\\n");
			}

			pos = (int32_t)content.find("\\t");
			while (pos != std::string::npos)
			{
				content[pos] = '\t';
				content.replace(pos + 1, 1, "");//erase a char
				pos = content.find("\\t");
			}

			pos = (int32_t)content.find("\\r");
			while (pos != std::string::npos)
			{
				content[pos] = '\r';
				content.replace(pos + 1, 1, "");//erase a char
				pos = content.find("\\r");
			}

            std::cout << content;
			return nullptr;
		};
    }

    DataStructure::DataStructure(VM *vm)
        : Library(vm)
    {
        m_NativeFunctions["sizeof"] = [this](std::vector<Object *> args) -> Object *
        {
            if (args.empty() || args.size() > 1)
                Assert("[Native function 'sizeof']:Expect a argument.");

            if (IS_ARRAY_OBJ(args[0]))
                return m_VMHandle->CreateIntegerObject(TO_ARRAY_OBJ(args[0])->elements.size());
            else if(IS_TABLE_OBJ(args[0]))
                return m_VMHandle->CreateIntegerObject(TO_TABLE_OBJ(args[0])->elements.size());
            else if(IS_STR_OBJ(args[0]))
                return m_VMHandle->CreateIntegerObject(TO_STR_OBJ(args[0])->value.size());
            else 
                Assert("[Native function 'sizeof']:Expect a array,table ot string argument.");
            return nullptr;
        };

        m_NativeFunctions["insert"] = [this](std::vector<Object*> args)->Object*
        {
			if (args.empty() || args.size() !=3)
				Assert("[Native function 'insert']:Expect 3 arguments,the arg0 must be array,table or string object.The arg1 is the index object.The arg2 is the value object.");

            if (IS_ARRAY_OBJ(args[0]))
            {
                ArrayObject* array = TO_ARRAY_OBJ(args[0]);
                if (!IS_INTEGER_OBJ(args[1]))
                    Assert("[Native function 'insert']:Arg1 must be integer type while insert to a array");

                int64_t iIndex = TO_INTEGER_OBJ(args[1])->value;

                if(iIndex<0||iIndex>=array->elements.size())
                    Assert("[Native function 'insert']:Index out of array's range");

                array->elements.insert(array->elements.begin()+iIndex,1,args[2]);
            }
            else if (IS_TABLE_OBJ(args[0]))
            {
				TableObject* table = TO_TABLE_OBJ(args[0]);
			
                for (auto [key, value] : table->elements)
                    if (key->IsEqualTo(args[1]))
                        Assert("[Native function 'insert']:Already exist value in the table object of arg1"+args[1]->Stringify());

                table->elements[args[1]]=args[2];
            }
            else if (IS_STR_OBJ(args[0]))
            {
				StrObject* string = TO_STR_OBJ(args[0]);
				if (!IS_INTEGER_OBJ(args[1]))
					Assert("[Native function 'insert']:Arg1 must be integer type while insert to a array");

				int64_t iIndex = TO_INTEGER_OBJ(args[1])->value;

				if (iIndex < 0 || iIndex >= string->value.size())
					Assert("[Native function 'insert']:Index out of array's range");

				string->value.insert(iIndex,args[2]->Stringify());
            }
			else
				Assert("[Native function 'insert']:Expect a array,table ot string argument.");
			return nullptr;
        };


		m_NativeFunctions["erase"] = [this](std::vector<Object*> args)->Object*
		{
			if (args.empty() || args.size() != 2)
				Assert("[Native function 'erase']:Expect 2 arguments,the arg0 must be array,table or string object.The arg1 is the corresponding index object.");

			if (IS_ARRAY_OBJ(args[0]))
			{
				ArrayObject* array = TO_ARRAY_OBJ(args[0]);
				if (!IS_INTEGER_OBJ(args[1]))
					Assert("[Native function 'erase']:Arg1 must be integer type while insert to a array");

				int64_t iIndex = TO_INTEGER_OBJ(args[1])->value;

				if (iIndex < 0 || iIndex >= array->elements.size())
					Assert("[Native function 'erase']:Index out of array's range");

				array->elements.erase(array->elements.begin()+iIndex);
			}
			else if (IS_TABLE_OBJ(args[0]))
			{
				TableObject* table = TO_TABLE_OBJ(args[0]);

				bool hasValue = false;

				for (auto it = table->elements.begin(); it != table->elements.end(); ++it)
					if (it->first->IsEqualTo(args[1]))
					{
						table->elements.erase(it);
						hasValue = true;
						break;
					}

				if (!hasValue)
					Assert("[Native function 'erase']:No corresponding index in table.");
			}
			else if (IS_STR_OBJ(args[0]))
			{
				StrObject* string = TO_STR_OBJ(args[0]);
				if (!IS_INTEGER_OBJ(args[1]))
					Assert("[Native function 'erase']:Arg1 must be integer type while insert to a array");

				int64_t iIndex = TO_INTEGER_OBJ(args[1])->value;

				if (iIndex < 0 || iIndex >= string->value.size())
					Assert("[Native function 'erase']:Index out of array's range");

				string->value.erase(string->value.begin()+iIndex);
			}
			else
				Assert("[Native function 'erase']:Expect a array,table ot string argument.");
			return nullptr;
		};
    }
}