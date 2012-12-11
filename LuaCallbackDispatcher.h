#pragma once
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

///@file
///@brief Qt signals to Lua callback functions.  

#include <QMap>
#include <QList>

#include "LuaArguments.h"


namespace qlua {

class LuaContext;

typedef QList< LArgWrapper > CBackParameterTypes;

//------------------------------------------------------------------------------
/// @brief C++ method abstraction: Qt signals are connected to instances of this
/// class which invokes associated Lua function through the @c Invoke method.
///
/// At signal connection time a signal is connected to a dynamically created
/// instance of this class which stores internally a reference to the Lua
/// function to invoke. 
class LuaCBackMethod {
public:
    /// @brief Constructor
    /// @param lc LuaContext
    /// @param p signal signature: This information is used to translate
    ///          the parameter values received from the signal (as an array of void*)
    ///          into Lua values
    /// @param luaCBackRef reference (as a Lua integer reference) to Lua function
    ///                    to invoke
    LuaCBackMethod( LuaContext* lc, const CBackParameterTypes& p, int luaCBackRef ) 
        : lc_( lc ), paramTypes_( p ), luaCBackRef_( luaCBackRef ) {}
    /// @brief Called by QObject::qt_metacall as part of a signal-method invocation. 
    ///
    /// Iterates over the list of arguments and parameter types in parallel and
    /// for each argument uses the corresponding parameter wrapper to 
    /// translate and push values onto the Lua stack.
    /// Values which are of QObject* type are automatically translated to Lua
    /// table.
    /// When a QObject is added to the Lua context its life-time is not
    /// managed by Lua.
    void Invoke( void **arguments );
    /// Return associated reference to Lua function
    int CBackRef() const{ return luaCBackRef_; } 
private:
    /// LuaContext instance
    LuaContext* lc_;
    /// Signature of associated signal
    CBackParameterTypes paramTypes_;
    /// Reference to Lua function to invoke
    int luaCBackRef_;
};


typedef int LuaCBackRef;
typedef int MethodId;

//------------------------------------------------------------------------------
/// @brief Manages Lua function invocation through Qt signals. And connection
/// of Qt signals to Lua functions or QObject methods.
///
/// Offers methods to connect Qt signals emitted from QObjects to Lua functions
/// or other QObject methods.
/// Whenever a new signal -> Lua connection is requested a new proxy method is
/// generated and the signal is routed to the new method which in turn takes
/// care of invoking the Lua function.
/// Note that when disconnecting a signal the associated method is not currently
/// removed from the method array because signals are connected to methods through
/// the method's position in the method array, thus removing a method from the array
/// invalidates all the signal to method connections for which the method index
/// is greater than the one of the removed method.
class LuaCallbackDispatcher : public QObject {
public:
    /// Standard QObject constructor
    LuaCallbackDispatcher( QObject* parent = 0 ) : QObject( parent ), lc_( 0 ) {}
    /// Constructor, bind dispatcher to Lua context
    LuaCallbackDispatcher( LuaContext* lc ) : lc_( lc ) {}
    /// Overridden method: This is what makes it possible to bind a signal
    /// to a Lua function through the index of a proxy method.
    int qt_metacall( QMetaObject::Call c, int id, void **arguments ); 
    /// Connect signal to Lua function
    /// @param obj source QObject
    /// @param signalIdx signal index
    /// @param paramTypes signal signature
    /// @param luaCBackRef reference to Lua target function created through @c luaL_ref
    bool Connect( QObject *obj, 
                  int signalIdx,
                  const CBackParameterTypes& paramTypes,
                  int luaCBackRef );
    /// Disconnect signal from Lua function; function must be already on the stack
    /// @param obj source QObject
    /// @param signalIdx signal index
    /// @param cbackStackIndex position of Lua function in Lua stack
    bool Disconnect( QObject *obj, 
                     int signalIdx,
                     int cbackStackIndex );
    /// Set LuaContext
    void SetLuaContext( LuaContext* lc ) { lc_ = lc; };
    /// Destructor: Clear method database
    virtual ~LuaCallbackDispatcher() {
        for( QList< LuaCBackMethod* >::iterator i = luaCBackMethods_.begin();
             i != luaCBackMethods_.end(); ++i ) {
            delete *i;
        }
    }
private:
    /// LuaContext
    LuaContext* lc_;
    /// Methods
    QList< LuaCBackMethod* > luaCBackMethods_;
    /// Map Lua reference to method index in luaCBackMethods_ list
    QMap< LuaCBackRef, MethodId > cbackToMethodIndex_;
   
};
}
