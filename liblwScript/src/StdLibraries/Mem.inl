
const auto AddressOfFunction = new NativeFunctionObject([](Value *args, uint32_t argCount, const Token *relatedToken, Value &result) -> bool
                                                        {
                                                            if (args == nullptr || argCount != 1)
                                                                Hint::Error(relatedToken, L"[Native function 'addressof']:Expect 1 arguments.");

                                                            if (!IS_OBJECT_VALUE(args[0]))
                                                                Hint::Error(relatedToken, L"[Native function 'addressof']:The arg0 is a value,only object has address.");

                                                            result = new StrObject(PointerAddressToString(args[0].object));
                                                            return true;
                                                        });

auto memClass = new ClassObject(L"mem");
memClass->members[L"addressof"] = AddressOfFunction;
mLibraries.emplace_back(memClass);