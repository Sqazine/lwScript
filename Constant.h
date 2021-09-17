#pragma once
#include "Ast.h"
#include "Object.h"
namespace lwScript
{
    static NilExpr *nilExpr = new NilExpr();
    static BoolExpr *trueExpr = new BoolExpr(true);
    static BoolExpr *falseExpr = new BoolExpr(false);

    static BoolObject *trueObject = new BoolObject(true);
    static BoolObject *falseObject = new BoolObject(false);
    static NilObject *nilObject = new NilObject();
}