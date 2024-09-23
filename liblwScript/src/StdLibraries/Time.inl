
const auto ClockFunction = new NativeFunctionObject([](Value *args, uint32_t argCount, const Token *relatedToken, Value &result) -> bool
                                                    {
                                                        result = Value((double)clock() / CLOCKS_PER_SEC);
                                                        return true;
                                                    });

auto timeClass = new ClassObject(L"time");
timeClass->members[L"clock"] = ClockFunction;
mLibraries.emplace_back(timeClass);