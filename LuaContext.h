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
///@brief Lua context: Creates or wraps an existing Lua state.  

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <stdexcept>
#include <string>

#include <QMetaMethod>
#include <QString>
#include <QMap>
#include <QList>
#include <QStringList>

#include "LuaCallbackDispatcher.h"
#include "LuaArguments.h"
#include "LuaQtTypes.h"
#include "ILuaSignatureMapper.h"

#define QLUA_VERSION "0.2"
#define QLUA_VERSION_MAJ 0
#define QLUA_VERSION_MIN 2

namespace qlua {

inline void RaiseLuaError( lua_State* L, const char* errMsg ) {
    lua_pushstring( L, errMsg );
    lua_error( L );
}
inline void RaiseLuaError( lua_State* L, const QString& errMsg ) {
    RaiseLuaError( L, errMsg.toAscii().constData() );
}
inline void RaiseLuaError( lua_State* L, const std::string& errMsg ) {
    RaiseLuaError( L, errMsg.c_str() );
}

/// @brief Default mapper for method signature; returns name of method
struct LuaDefaultSignatureMapper : ILuaSignatureMapper {
    /// Extract and return method name
    /// @param signature method signature
    /// @return method name 
    QString map( const QString& sig ) const {
        QString name = sig;
        name.truncate( sig.indexOf( "(" ) );
        return name;
    }
};


//------------------------------------------------------------------------------
/// @brief Lua context. Creates or wraps an existing Lua state.
///
/// This class is the interface exposed by QLua to client code.
/// Use the provided method to add QObjects and other types the Lua context and
/// to evaluate Lua code.
/// LuaContext is also used internally by other classes to add QObjects returned
/// by methods or received from signals to the Lua context.
class LuaContext {
    /// @brief Stores information used at method invocation time.
    /// 
    /// When a new QObject is added to the Lua context a new Method is created
    /// for each callable method (i.e. slot or Q_INVOKABLE) storing the signature
    /// to be used at invocation time and the QMetaMethod to use for the actual
    /// invocation.
    struct Method {
        QObject* obj_;
        QMetaMethod metaMethod_;
        QArgWrappers argumentWrappers_;
        LArgWrapper returnWrapper_;
        Method( QObject* obj, const QMetaMethod& mm, const QArgWrappers& pw, const LArgWrapper& rw ) :
        obj_( obj ), metaMethod_( mm ), argumentWrappers_( pw ), returnWrapper_( rw ) {}
    };
public:
    /// Delete mode: Specify how/if object shall be garbage collected
    enum ObjectDeleteMode { 
        QOBJ_NO_DELETE, ///< Lifetime not managed by Lua; never garbage collected 
        QOBJ_IMMEDIATE_DELETE, ///< Garbage collected: @e delete used
        QOBJ_DELETE_LATER ///< Garbage collected: @e QObject::deleteLater() used
    };
    typedef QList< Method > Methods;
    typedef QMap< QObject*, QMap< QString, Methods > > ObjectMethodMap;
    typedef QMap< QObject*, int > ObjectReferenceMap;
    /// Constructor: Create @c qlua table with QLua interface.
    /// @param L if not null the passed Lua state is used, otherwise a new one is created.
    LuaContext( lua_State* L = 0 ); 
    /// Return Lua state.
    lua_State* LuaState() const { return L_; }
    /// Evaluate Lua code.
    void Eval( const char* code ) {
        ReportErrors( luaL_dostring( L_, code ) );
    }
    /// @brief Add QVariantMap: Either push it on the stack or set it as global.
    /// @param vm QVariantMap
    /// @param name global name; if null value is left on the Lua stack.
    void AddQVariantMap( const QVariantMap& vm, const char* name = 0 ) {        
        VariantMapToLuaTable( vm, L_ );
        if( name ) lua_setglobal( L_, name );
    }
    /// @brief Add QVariantList: Either push it on the stack or set it as global.
    /// @param vl QVariantList
    /// @param name global name; if null value is left on the Lua stack.
    void AddQVariantList( const QVariantList& vl, const char* name = 0 ) {      
        VariantListToLuaTable( vl, L_ );
        if( name ) lua_setglobal( L_, name );
    }
    /// @brief Add QStringList: Either push it on the stack or set it as global.
    /// @param sl QStringList
    /// @param name global name; if null value is left on the Lua stack.
    void AddQStringList( const QStringList& sl, const char* name = 0 ){     
        StringListToLuaTable( sl, L_ );
        if( name ) lua_setglobal( L_, name );
    }
    /// @brief Add QList of numeric values: Either push it on the stack or set it as global.
    /// @param l QList
    /// @param name global name; if null value is left on the Lua stack.
    template < typename T > void AddQList( const QList< T >& l, const char* name = 0 ) {     
        NumberListToLuaTable< T >( l, L_ );
        if( name ) lua_setglobal( L_, name );
    }
    /// @brief Add QObject to Lua context as a Lua table.
    ///
    /// When a new QObject is added this method:
    ///   -# adds a new QObject reference in the QObject-Method database
    ///   -# iterates over the callable QObject's methods and for each method
    ///      adds a Method object with information required to invoke the QObject method
    ///   -# if caching is enabled it creates a Lua reference and adds the reference
    ///      into the QObject-Reference database
    /// @param obj QObject
    /// @param tableName global name of Lua table wrapping object; if null object is
    ///                  left on stack
    /// @param cache if true object won't be re-added to LuaContext. If @c tableName is
    ///              not null a new global variable pointing at the previoulsy added object will be added.
    /// @param mapper maps signature string to Lua method name; this allows to convert overloaed methods
    ///               to different Lua functions.
    /// @param deleteMode choose how/if object shall be garbage collected; @see ObjectDeleteMode
    /// @param methodNames if not empty only the methods with the names in this list are added to the Lua table
    /// @param methodTypes if not empty only the methods of the required types are added to the Lua table  
    void AddQObject( QObject* obj, 
                     const char* tableName = 0,
                     bool cache = false, 
                     ObjectDeleteMode deleteMode = QOBJ_NO_DELETE,
                     const ILuaSignatureMapper& mapper = LuaDefaultSignatureMapper(),
                     const QStringList& methodNames = QStringList(),
                     const QList< QMetaMethod::MethodType >& methodTypes =
                           QList< QMetaMethod::MethodType >()  );
    /// @brief Return value of global garbage collection policy.
    /// 
    /// The global object ownership policy is set from Lua through a call to
    /// @c qlua.ownQObjects(). The ownership policy affects the QObjects returned
    /// by QObject methods only.
    bool OwnQObjects() const { return ownQObjects_; }
    /// Destructor: Destroys Lua state if owned by this object
    ~LuaContext() {
        if( !wrappedContext_ ) lua_close( L_ );
    }
private:
    /// Remove object from databases.
    void RemoveObject( QObject* obj );
    /// @name Lua interface
    //@{
    /// Connect Qt signal to Lua function or QObject method
    static int QtConnect( lua_State* L );
    /// Disconnect Qt signal from Lua function or QObject method
    static int QtDisconnect( lua_State* L );
    /// Invoke QObject method, this is the function that is called
    /// by each Lua function added to the QObject table: information
    /// on QObject instance and method to call are extracted from 
    /// closure environment as upvalues
    static int InvokeMethod( lua_State* L );
    /// Invoked automatically by Lua when value is garbage collected 
    static int DeleteObject( lua_State* L );
    /// Set default policy for ownership of returned QObjects
    static int SetQObjectsOwnership( lua_State* L );
    //@}
    //@{
    /// Called by Invoke depending on the number of arguments in
    /// method signature.
    static int Invoke0( const Method* mi, LuaContext& L );
    static int Invoke1( const Method* mi, LuaContext& L );
    static int Invoke2( const Method* mi, LuaContext& L );
    static int Invoke3( const Method* mi, LuaContext& L );
    static int Invoke4( const Method* mi, LuaContext& L );
    static int Invoke5( const Method* mi, LuaContext& L );
    static int Invoke6( const Method* mi, LuaContext& L );
    static int Invoke7( const Method* mi, LuaContext& L );
    static int Invoke8( const Method* mi, LuaContext& L );
    static int Invoke9( const Method* mi, LuaContext& L );
    static int Invoke10( const Method* mi, LuaContext& L );
    //@}
    /// Push error message on Lua stack and trigger a Lua error.
    void ReportErrors( int status ) {
        if( status != 0 ) {
            std::string err = lua_tostring( L_, -1 );
            lua_pop( L_, 1 );
            throw std::runtime_error( err );
        }
    }
    /// Register supported types.
    static void RegisterTypes();
private:
    /// (possibly wrapped) Lua state
    lua_State* L_;
    /// Signal if context is wrapped or owned
    bool wrappedContext_;   
    /// State variable affecting the life-time management of returned QObjects
    bool ownQObjects_;
    /// @brief QObject-Method database: Each QObject is stored together with the list
    /// of associated method signatures
    ObjectMethodMap objMethods_;
    /// QObject-Lua reference database  
    ObjectReferenceMap objRefs_;
    /// @brief Dispatcher object: signal->dispatcher->Lua function connection.
    ///
    /// Each time a connection between a Qt signal and a Lua function is requested
    /// a new connection is established between the signal and a dynamically created
    /// proxy method which invokes the Lua function.
    LuaCallbackDispatcher dispatcher_;
};


/// Extract C++ value from Lua context.
/// @tparam T type of returned value
/// @param lc LuaContext
/// @param name global name of variable in Lua context
template < typename T >
T GetValue( const LuaContext& lc, const QString& name ) {
    lua_getglobal( lc.LuaState(), name.toAscii().constData() );
    return luaL_checknumber( lc.LuaState(), -1 );
}

/// Extract list of number.
template < typename T >
QList< T > GetValues( const LuaContext& lc, const QString& name ) {
    if( !lua_istable( lc.LuaState(), -1 ) ) throw std::runtime_error( "Not a lua table" );
    return ParseLuaTableAsNumberList< T >( lc.LuaState(), -1 );
}

/// Extract string.
template <>
inline QString GetValue< QString >( const LuaContext& lc, const QString& name ) {
    lua_getglobal( lc.LuaState(), name.toAscii().constData() );
    return luaL_checkstring( lc.LuaState(), -1 );
}

/// Extract Lua table as variant map.
template <>
inline QVariantMap GetValue< QVariantMap >( const LuaContext& lc, const QString& name ) {
    lua_getglobal( lc.LuaState(), name.toAscii().constData() );
    if( !lua_istable( lc.LuaState(), -1 ) ) throw std::runtime_error( "Not a lua table" );
    return ParseLuaTable( lc.LuaState(), -1 );
}

/// Extract Lua table as variant list.
template <>
inline QVariantList GetValue< QVariantList >( const LuaContext& lc, const QString& name ) {
    lua_getglobal( lc.LuaState(), name.toAscii().constData() );
    if( !lua_istable( lc.LuaState(), -1 ) ) throw std::runtime_error( "Not a lua table" );
    return ParseLuaTableAsVariantList( lc.LuaState(), -1 );
}

/// Extract Lua table as string list.
template <>
inline QStringList GetValue< QStringList >( const LuaContext& lc, const QString& name ) {
    lua_getglobal( lc.LuaState(), name.toAscii().constData() );
    if( !lua_istable( lc.LuaState(), -1 ) ) throw std::runtime_error( "Not a lua table" );
    return ParseLuaTableAsStringList( lc.LuaState(), -1 );
}

}
