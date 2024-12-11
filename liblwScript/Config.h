#pragma once
#include "Defines.h"
namespace lwscript
{
    class LWSCRIPT_API Config
    {
    public:
        SINGLETON_DECL(Config)

        void SetIsUseFunctionCache(bool v);
        bool IsUseFunctionCache() const;

        void SetIsUseConstantFold(bool v);
        bool IsUseConstantFold() const;

    private:
        Config() = default;
        ~Config() = default;

        bool mUseFunctionCache{false};
        bool mUseConstandFold{false};
    };
}