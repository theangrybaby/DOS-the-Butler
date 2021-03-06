// Written in the D programming language.

/**
Functions that manipulate other functions.

Macros:

WIKI = Phobos/StdFunctional

Author:

$(WEB erdani.org, Andrei Alexandrescu)
*/

/*
 *  Copyright (C) 2004-2006 by Digital Mars, www.digitalmars.com
 *  Written by Andrei Alexandrescu, www.erdani.org
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  o  The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  o  Altered source versions must be plainly marked as such, and must not
 *     be misrepresented as being the original software.
 *  o  This notice may not be removed or altered from any source
 *     distribution.
 */

module std.functional;

import std.string; // for making string functions visible in *naryFun
import std.conv; // for making conversions visible in *naryFun
import std.typetuple;
import std.typecons;
import std.stdio;
import std.metastrings;

/**
Transforms a string representing an expression into a unary
function. The string must use symbol name $(D a) as the parameter.

Example:

----
alias unaryFun!("(a & 1) == 0") isEven;
assert(isEven(2) && !isEven(1));
----
*/

template unaryFun(string comp, bool byRef = false) {
    static if (byRef)
    {
        void unaryFun(ElementType)(ref ElementType a)
        {
            mixin(comp ~ ";");
        }
    }
    else
    {
        // @@@BUG1816@@@: typeof(mixin(comp)) should work
        typeof({ ElementType a; return mixin(comp);}())
            unaryFun(ElementType)(ElementType a)
        {
            return mixin(comp);
        }
    }
}

/**
Transforms a string representing an expression into a Boolean binary
predicate. The string must use symbol names $(D a) and $(D b) as the
compared elements.

   Example:

----
alias binaryFun!("a < b") less;
assert(less(1, 2) && !less(2, 1));
alias binaryFun!("a > b") greater;
assert(!greater("1", "2") && greater("2", "1"));
----
*/

template binaryFun(string comp)
{
    // @@@BUG1816@@@: typeof(mixin(comp)) should work
    typeof({
            static ElementType1 a;
            static ElementType2 b;
            return mixin(comp);
        }())
    binaryFun(ElementType1, ElementType2)(ElementType1 a, ElementType2 b)
    {
        return mixin(comp);
    }
}

unittest
{
    alias binaryFun!(q{a < b}) less;
    assert(less(1, 2) && !less(2, 1));
    assert(less("1", "2") && !less("2", "1"));
}

/*
   Predicate that returns $(D_PARAM a < b).
*/
//bool less(T)(T a, T b) { return a < b; }
//alias binaryFun!(q{a < b}) less;

/*
   Predicate that returns $(D_PARAM a > b).
*/
//alias binaryFun!(q{a > b}) greater;

/*
   Predicate that returns $(D_PARAM a == b).
*/
//alias binaryFun!(q{a == b}) equalTo;

/*
   Binary predicate that reverses the order of arguments, e.g., given
   $(D pred(a, b)), returns $(D pred(b, a)).
*/
template binaryRevertArgs(alias pred)
{
    typeof({ ElementType1 a; ElementType2 b; return pred(b, a);}())
    binaryRevertArgs(ElementType1, ElementType2)(ElementType1 a, ElementType2 b)
    {
        return pred(b, a);
    }
}

unittest
{
    alias binaryRevertArgs!(binaryFun!("a < b")) gt;
    assert(gt(2, 1) && !gt(1, 1));
    int x = 42;
    bool xyz(int a, int b) { return a * x < b / x; }
    auto foo = &xyz;
    foo(4, 5);
    alias binaryRevertArgs!(foo) zyx;
    assert(zyx(5, 4) == foo(4, 5));
}

template not(alias pred)
{
    bool not(T...)(T args) { return !pred(args); }
}

/*private*/ template Adjoin(F...)
{
    template For(V...)
    {
        static if (F.length == 0)
        {
            alias TypeTuple!() Result;
        }
        else
        {
            alias F[0] headFun;
            alias typeof({ V values; return headFun(values); }()) Head;
            alias TypeTuple!(Head, Adjoin!(F[1 .. $]).For!(V).Result) Result;
        }
    }
}

/**
Takes multiple functions and adjoins them together. The result is a
$(XREF typecons, Tuple) with one element per passed-in function. Upon
invocation, the returned tuple is the adjoined results of all
functions.

Example:

----
static bool f1(int a) { return a != 0; }
static int f2(int a) { return a / 2; }
auto x = adjoin!(f1, f2)(5);
assert(is(typeof(x) == Tuple!(bool, int)));
assert(x._0 == true && x._1 == 2);
----
*/
template adjoin(F...)
{
    Tuple!(Adjoin!(F).For!(V).Result) adjoin(V...)(V a)
    {
        typeof(return) result;
        foreach (i, r; F)
        {
            // @@@BUG@@@
            mixin("result.field!("~ToString!(i)~") = F[i](a);");
        }
        return result;
    }
}

unittest
{
    static bool F1(int a) { return a != 0; }
    static int F2(int a) { return a / 2; }
    auto x = adjoin!(F1, F2)(5);
    alias Adjoin!(F1, F2).For!(int).Result R;
    assert(is(typeof(x) == Tuple!(bool, int)));
    assert(x._0 == true && x._1 == 2);
}

// /*private*/ template NaryFun(string fun, string letter, V...)
// {
//     static if (V.length == 0)
//     {
//         enum args = "";
//     }
//     else
//     {
//         enum args = V[0].stringof~" "~letter~"; "
//             ~NaryFun!(fun, [letter[0] + 1], V[1..$]).args;
//         enum code = args ~ "return "~fun~";";
//     }
//     alias void Result;
// }

// unittest
// {
//     writeln(NaryFun!("a * b * 2", "a", int, double).code);
// }

// /**
// naryFun 
//  */
// template naryFun(string fun)
// {
//     //NaryFun!(fun, "a", V).Result
//     int naryFun(V...)(V values)
//     {
//         enum string code = NaryFun!(fun, "a", V).code;
//         mixin(code);
//     }
// }

// unittest
// {
//     alias naryFun!("a + b") test;
//     test(1, 2);
// }

/**
Composes two functions $(D F1) and $(D F2), returning a function $(D
F(x)) that in turn returns $(D F1(F2(x))).

Example:
----
auto x = 5;
auto y = compose!("!a", "a > 0")(x);
assert(!y);
----
*/

template compose(alias F1, alias F2)
{
    typeof({ E a; return F1(F2(a)); }()) compose(E)(E a)
    {
        return F1(F2(a));
    }
}
/// Ditto
template compose(alias F1, string F2)
{
    alias .compose!(F1, unaryFun!(F2)) compose;
}
/// Ditto
template compose(string F1, alias F2)
{
    alias .compose!(unaryFun!(F1), F2) compose;
}
/// Ditto
template compose(string F1, string F2)
{
    alias .compose!(unaryFun!(F1), unaryFun!(F2)) compose;
}

