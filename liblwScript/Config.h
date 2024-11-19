#pragma once
#include "Defines.h"
namespace lwscript
{
    class LWSCRIPT_API Config
    {
        NON_COPYABLE(Config)
    public:
        SINGLETON_DECL(Config)

        void SetIsUseFunctionCache(bool v);
        bool IsUseFunctionCache() const;

    private:
        Config() = default;
        ~Config() = default;

        bool mUseFunctionCache{false};
    };
}