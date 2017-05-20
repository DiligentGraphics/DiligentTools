/*     Copyright 2015-2017 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#include "pch.h"
#include "ParsingErrors.h"

static Diligent::String FindSourceLine( const char *pSource, int LineNumber )
{
    const char *pLineStart = pSource;
    for( int CurrLine = 1; CurrLine < LineNumber; ++CurrLine )
    {
        pLineStart = strchr( pLineStart, '\n' );
        if( !pLineStart )
            break;
        ++pLineStart;
    }

    Diligent::String Line;
    if( pLineStart )
    {
        for( ; *pLineStart != '\n' && *pLineStart != 0; ++pLineStart )
            Line += *pLineStart;
    }
    return Line;
}

void LuaDebugInformation( lua_State *L, std::stringstream &ss )
{
    // Trace the Lua stack
    ss << "Lua stack:\n";
    
    Diligent::String FailureLine;
    //String ShortSource;

    lua_Debug info;
    int level = 0;
    //int FailureLineNum = -1;
    // lua_getstack() retrieves information about the interpreter runtime stack.
    // This function fills parts of a lua_Debug structure that is required to call lua_getinfo()
    // Level 0 is the current running function, whereas level n+1 is the function that has called 
    // level n (except for tail calls, which do not count on the stack). When there are no errors, 
    // lua_getstack returns 1; when called with a level greater than the stack depth, it returns 0.
    while( lua_getstack( L, level, &info ) )
    {
        // Get current stack level info

        // 'n': fills in the field name and namewhat;
        // 'S': fills in the fields source, short_src, linedefined, lastlinedefined, and what;
        // 'l': fills in the field currentline;
        // 't': fills in the field istailcall;
        // 'u': fills in the fields nups, nparams, and isvararg;
        // 'f': pushes onto the stack the function that is running at the given level;
        // 'L': pushes onto the stack a table whose indices are the numbers of the lines that are valid 
        //      on the function. (A valid line is a line with some associated code, that is, a line where 
        //      you can put a break point.Non - valid lines include empty lines and comments.)
        lua_getinfo( L, "nSl", &info );
        ss << (info.name ? info.name : "") << '<' << (info.what ? info.what : "") <<  "> Line " << info.currentline << '\n';
        if( info.currentline >= 0 && FailureLine.length() == 0 )
        {
            //ShortSource = info.short_src;
            FailureLine = FindSourceLine( info.source, info.currentline );
            //FailureLineNum = info.currentline;
        }
        ++level;
    }
    ss << "Failure line:\n" << FailureLine;// << "\nLocation:\n" << ShortSource << ':' << FailureLineNum;
}
