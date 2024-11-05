#include "Config.h"

namespace lwscript
{
    SINGLETON_IMPL(Config)

    void Config::SetIsUseFunctionCache(bool v)
    {
        mUseFunctionCache = v;
    }

    bool Config::IsUseFunctionCache() const
    {
        return mUseFunctionCache;
    }
}