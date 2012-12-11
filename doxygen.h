// DOXYGEN FRONT PAGE

//QLua - Copyright (c) 2012, Ugo Varetto
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the author and copyright holder nor the
//       names of contributors to the project may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL UGO VARETTO BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**@mainpage QLua

QLua is a Qt <--> Lua binding framework.

QObjects are exposed to Lua as tables with values matching QObject
properties and callable methods declared as signals, slots or
@c Q_INVOKABLE. 

The public interface is defined in the qlua::LuaContext class.

@section Usage

QLua is used to access exported QObject methods and properties from
Lua and to connect QObject signals to Lua functions or QObject slots.

@subsection Initialization

Create or wrap a @c lua_State with qlua::LuaContext constructor.
Add QObjects with qlua::LuaContext::AddQObject method.

@code

#include <LuaContext.h>
...
using namespace qlua;
...
LuaContext lc;  // create a new lua_State
LuaContext lcw( luaStatePtr ) ; // wraps an existing lua_State
...
MyQObject myobj;
lc.AddQObject( &myobj, "myobj" ); // add MyQObject instance to lua_State
...

@endcode

@subsection MethodInvocation Method invocation

Each QObject is added to Lua as a table, with the name specified in the qlua::LuaContext::AddQObject method invocation.
You invoke a method from Lua as a function stored inside a table.

@code
myobj.aMethod('anArgument');
@endcode

@endcode

@subsection Signals

Signals can be connected to
  -# Lua functions
  -# QObject methods

In the case of (1) a function is passed as the endpoint of a signal connection.
(2) requires both a target QObject and a method signature.

QObjects are passed to the @c qlua.connect function as QObject instances 
previously added through qlua::LuaContext::AddQObject or as pointers
added to Lua by calling @c lua_pushlightuserdata. 

@code
qlua.connect( myobj, 'aSignal(QString)',
              function( msg ) print( msg ) end );
myobj.emitSignal( 'hello' ); 
@endcode

@section MemoryManagement Memory management

QObjects added to Lua with qlua::LuaContext::AddQObject are not garbage collected
by default.

To have a QObject deleted when the Lua table is garbage collected set the value
of the @c deleteMode parameter in the invocation of qlua::LuaContext::AddQObject to one of:
  - QOBJ_IMMEDIATE_DELETE to have the QObject instance deleted with @c delete
  - QOBJ_DELETE_LATER to have the QObject instance deleted through @c QObject::deleteLater

@see qlua::LuaContext::ObjectDeleteMode 

The @c deleteMode parameter will be replaced in the future with a custom deleter.  

*/


