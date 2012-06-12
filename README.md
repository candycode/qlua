QLua
====

QLua is a binding between lua 5.1/5.2/luajit and Qt(tested with versions 
4.7 and 4.8).

http://candycode.github.com/qlua


License & Copyright
-------------------

QLua is Copyright (c) 2012 by Ugo Varetto and distributed under the terms of the
BSD three-clause license (i.e. do what you want with it, just please add 
copyright information).
Complete license information is included in the file 'LICENSE', part of this 
source code distribution.


Features
--------

- add QObjects to Lua context: QObject instances are translated to Lua tables,
  and invokable methods are automatically added as functions to the table;
- specify the subset of methods/method types to be added to the Lua table;
- invoke QObject methods exposed as signals, slots or through the Q_INVOKABLE
  macro;
- connect QObject signals to Lua callback functions;
- connect QObject signals to QObject methods;
- optionally have Lua destroy the added QObjects when tables are garbage
  collected;

QLua **is not** a Lua wrapper for the Qt toolkit; its main use is to
expose pre-created QObjects instances to the Lua environment.
It is however fairly easy to use QLua as a Qt toolkit wrapper, you could e.g.

1. Create a QObject factory and add it to the Lua context
2. Add methods to the QObject factory to create QObject wrappers which
   expose a signal/slot interface to Lua

Note that (2) is required to access QObject/QWidget features not exposed
through slots.

Usage
-----

Create and instance of `qlua::LuaContext` to create a new lua_State or wrap an
existing one.

Add QObjects through the `qlua::LuaContext::AddQObject` method.

QLua functions are available from Lua through the global `qlua` object:

    qlua.connect( <qobject>, <signal signature>, 
                  <lua callback> | <qobject, method> )
    qlua.disconnect( <qobject>, <signal signature>, 
                     <lua callback> | <qobject, method> )
    qlua.version

`<qobject>` can be a table created through `LuaContext::AddQObject` or a plain
QObject pointer. 

E.g.

```cpp
    #include "LuaContext.h"

    using namespace qlua;
    ...
    LuaContext lc; 
    // or wrap pre-existing context: 'LuaContext lc( luaStatePtr );'
    ...
    // add qobject to context  
    MyQObject qobj1;
    lc.AddQObject( &qobj, "qobj1" );
    ...
    // add object and destroy it through Lua garbage collector;
    // QOBJ_IMMEDIATE_DELETE invokes 'delete' on the QObject pointer,
    // QOBJ_DELETE_LATER invokes QObject::deleteLater()
    MyQObject* qobj2 = new MyQObject;
    lc.AddQObject( qobj, "qobj2", LuaContext::QOBJ_IMMEDIATE_DELETE );
    // if the object instance name is null the object will not be
    // added as a global object but simply left on top of the Lua stack
    ...
    // add object restricting the invokable methods to 'method1' and 'method2'
    MyQObject qobj3;
    lc.AddQObject( &qobj3, "qobj3", QStringList() << "method1" << "method2" );
    ...
    // connect signal to lua callback and evaluate Lua expressions
    lc.Eval( "qlua.connect( qobj1, 'aSignal(QString)',"
                            "function( msg ) print( msg ) end)" );
    lc.Eval( "qobj1.emitSignal( 'hello' )" ); 
```

Build
-----

A CMake configuration file is provided to build the library and sample code.
Since however there are no pre-compilation configuration steps you can very
easily copy and paste the source code directly into any project.

Being a binding between Qt and Lua the only dependencies are a Lua and Qt 
distribution.
You should be able to build QLua on any platform that works with Qt >= 4.7 
and Lua >= 5.1.
I am personally using QLua on the following platforms (64bit versions only):

- Windows 7
- MacOS X Snow Leopard
- Ubuntu Linux 11.x

with Qt 4.8 and Lua versions 5.1.4, 5.2 as well as luajit-beta9.


Supported types
---------------

The currently supported types are:

- QVariantList
- QVariantMap
- QString
- QStringList
- QVector< int | short | float | double >
- QList< int | short | float | double >
- bool, int, float, double, short, long long
- void pointer
- QObject pointer, QWidget pointer

QVariantList and QVariantMap are converted to/from a Lua table.

QList<T> and QVector<T> are converted to/from a Lua table through
`lua_rawseti/lua_rawgeti`, so conversion is faster but metamethods are
not invoked.

Adding additional types
-----------------------

The current version of QLua does not support addition of new types in a
programmatic way i.e. you have to modify the source code yourself.

To add an additional type:

1. register the type through a call to qRegisterMetaType inside the 
   LuaContext::RegisterTypes() function in LuaContext.cpp;
2. add an ArgumentConstructor(LuaArguments.h) specialization to create a type
   instance from the data on the Lua stack;
3. add a ReturnConstructor(LuaArguments.h) specialization to create a Lua
   value from a type instance;
4. Add code to create the proper ArgumentConstructor/ReturnConstructor
   from the type name inside ParameterWrapper and ReturnWrapper constructors  

In general just have a look at how the various QList<T> and QVector<T> types
were added to qlua.

Limitations
-----------

The main limitation is that the resolution of overloaded methods is based
only on the number of parameters. 
i.e. qlua is not able to resolve a call to two methods like:

- MyObject::method( int );
- MyObject::method( double );

This is due to the fact that when invoking a C++ method from a dynamic language
it is not possible to automatically resolve the parameter types.
One option to address the issue is to find the best match at invocation time,
which has proven to be quite slow in practice and probably quite unsafe as well
since basically in the situation depicted above all numbers without a mantissa
would automatically be passed to the (int) method and the others to the (double)
one, which might or might not be correct.

It is possible to use custom mappers to translate overloaded methods to different
Lua functions. Have a look at the qlua::ILuaSignatureMapper and
qlua::LuaDefaultSignatureMapper classes for an explanation of the exposed interface.


Todo
----

- docs
- tests
- make it easier to register new user-defined types
- wrap QObject::tr()
- add additional pre-registered types namely:
  * low level arrays ( `struct Array { int size; T* data; }` )
  * QRegExp, QDate
