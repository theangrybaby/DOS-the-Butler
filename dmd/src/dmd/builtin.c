
// Compiler implementation of the D programming language
// Copyright (c) 1999-2007 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// http://www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "mars.h"
#include "declaration.h"
#include "attrib.h"
#include "expression.h"
#include "scope.h"
#include "mtype.h"
#include "aggregate.h"
#include "identifier.h"
#include "id.h"
#include "module.h"

/**********************************
 * Determine if function is a builtin one.
 */
enum BUILTIN FuncDeclaration::isBuiltin()
{
    static const char FeZe[] = "FeZe";	// real function(real)

    //printf("FuncDeclaration::isBuiltin() %s\n", toChars());
    if (builtin == BUILTINunknown)
    {
	builtin = BUILTINnot;
	if (parent && parent->isModule())
	{
	    if (parent->ident == Id::math &&
		parent->parent && parent->parent->ident == Id::std &&
		!parent->parent->parent)
	    {
		if (strcmp(type->deco, FeZe) == 0)
		{
		    if (ident == Id::sin)
			builtin = BUILTINsin;
		    else if (ident == Id::cos)
			builtin = BUILTINcos;
		    else if (ident == Id::tan)
			builtin = BUILTINtan;
		    else if (ident == Id::_sqrt)
			builtin = BUILTINsqrt;
		    else if (ident == Id::fabs)
			builtin = BUILTINfabs;
		    //printf("builtin = %d\n", builtin);
		}
	    }
	}
    }
    return builtin;
}


/**************************************
 * Evaluate builtin function.
 * Return result; NULL if cannot evaluate it.
 */

Expression *eval_builtin(enum BUILTIN builtin, Expressions *arguments)
{
    assert(arguments && arguments->dim);
    Expression *arg0 = (Expression *)arguments->data[0];
    Expression *e = NULL;
    switch (builtin)
    {
	case BUILTINsin:
	    if (arg0->op == TOKfloat64)
		e = new RealExp(0, sinl(arg0->toReal()), Type::tfloat80);
	    break;

	case BUILTINcos:
	    if (arg0->op == TOKfloat64)
		e = new RealExp(0, cosl(arg0->toReal()), Type::tfloat80);
	    break;

	case BUILTINtan:
	    if (arg0->op == TOKfloat64)
		e = new RealExp(0, tanl(arg0->toReal()), Type::tfloat80);
	    break;

	case BUILTINsqrt:
	    if (arg0->op == TOKfloat64)
		e = new RealExp(0, sqrtl(arg0->toReal()), Type::tfloat80);
	    break;

	case BUILTINfabs:
	    if (arg0->op == TOKfloat64)
		e = new RealExp(0, fabsl(arg0->toReal()), Type::tfloat80);
	    break;
    }
    return e;
}
