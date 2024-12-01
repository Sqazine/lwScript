#pragma once
#include "ConstantFolder.h"
#include "Ast.h"
#include "Config.h"
namespace lwscript
{
    class ParseOptimizer
    {
    public:
        ParseOptimizer() = default;
        ~ParseOptimizer() = default;

        Stmt *ParseOptimize(Stmt *stmt)
        {
            if (Config::GetInstance()->IsUseConstantFold())
                stmt = mConstantFolder.Fold(stmt);
            return stmt;
        }

    private:
        ConstantFolder mConstantFolder;
    };
}