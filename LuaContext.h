#pragma once
// QLua - Copyright (c) 2012, Ugo Varetto
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the <organization> nor the
//       names of its contributors may be used to endorse or promote products
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


//------------------------------------------------------------------------------
class LuaContext {
    struct Method {
        QObject* obj_;
        QMetaMethod metaMethod_;
        QArgWrappers argumentWrappers_;
        LArgWrapper returnWrapper_;
        Method( QObject* obj, const QMetaMethod& mm, const QArgWrappers& pw, const LArgWrapper& rw ) :
        obj_( obj ), metaMethod_( mm ), argumentWrappers_( pw ), returnWrapper_( rw ) {}
    };
public:
    enum ObjectDeleteMode { QOBJ_NO_DELETE, QOBJ_IMMEDIATE_DELETE, QOBJ_DELETE_LATER };
    typedef QList< Method > Methods;
    typedef QMap< QObject*, QMap< QString, Methods > > ObjectMethodMap;
    typedef QMap< QObject*, int > ObjectReferenceMap;
    LuaContext( lua_State* L = 0 ) : L_( L ), wrappedContext_( false ), ownQObjects_( false ) {
        
        if( L_ == 0 ) L_ = luaL_newstate();
        else wrappedContext_ = true;
        
        luaL_openlibs( L_);
        lua_newtable(L_);

        lua_pushstring( L_,  "connect" );
        lua_pushlightuserdata( L_, this );
        lua_pushcclosure( L_, &LuaContext::QtConnect , 1);
        lua_settable( L_, -3 );
        
        lua_pushstring( L_,  "disconnect" );
        lua_pushlightuserdata( L_, this );
        lua_pushcclosure( L_, &LuaContext::QtDisconnect , 1);
        lua_settable( L_, -3 );

        lua_pushstring( L_,  "ownQObjects" );
        lua_pushlightuserdata( L_, this );
        lua_pushcclosure( L_, &LuaContext::SetQObjectsOwnership , 1);
        lua_settable( L_, -3 );

        lua_pushstring( L_, "version" );
        lua_pushstring( L_, QLUA_VERSION );
        lua_settable( L_, -3 );

        lua_setglobal( L_, "qlua" );
        dispatcher_.SetLuaContext( this );
        RegisterTypes();
    }
    lua_State* LuaState() const { return L_; }
    void Eval( const char* code ) {
        ReportErrors( luaL_dostring( L_, code ) );
    }
    void AddQVariantMap( const QVariantMap& vm, const char* name = 0 ) {        
        VariantMapToLuaTable( vm, L_ );
        if( name ) lua_setglobal( L_, name );
    }
    void AddQVariantList( const QVariantList& vl, const char* name = 0 ) {      
        VariantListToLuaTable( vl, L_ );
        if( name ) lua_setglobal( L_, name );
    }
    void AddQStringList( const QStringList& sl, const char* name = 0 ){     
        StringListToLuaTable( sl, L_ );
        if( name ) lua_setglobal( L_, name );
    }
    template < typename T > void AddQList( const QList< T >& l, const char* name = 0 ){     
        NumberListToLuaTable< T >( l, L_ );
        if( name ) lua_setglobal( L_, name );
    }
    void AddQObject( QObject* obj, 
                     //not setting a global name allows to use this method to push a table on the stack
                     const char* tableName = 0,
                     bool cache = false, //caches objects: only one table per QObject pointer generated
                     ObjectDeleteMode deleteMode = QOBJ_NO_DELETE, //destroys QObject instance
                     const QStringList& methodNames = QStringList(),
                     const QList< QMetaMethod::MethodType >& methodTypes =
                         QList< QMetaMethod::MethodType >()  );
    bool OwnQObjects() const { return ownQObjects_; }
    ~LuaContext() {
        if( !wrappedContext_ ) lua_close( L_ );
    }
private:
    void RemoveObject( QObject* obj );
    static int QtConnect( lua_State* L );
    static int QtDisconnect( lua_State* L );
    static int DeleteObject( lua_State* L );
    static int InvokeMethod( lua_State* L );
    static int SetQObjectsOwnership( lua_State* L );
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
    void ReportErrors( int status ) {
        if( status != 0 ) {
            std::string err = lua_tostring( L_, -1 );
            lua_pop( L_, 1 );
            throw std::runtime_error( err );
        }
    }
    static void RegisterTypes();
private:
    lua_State* L_;
    bool wrappedContext_;   
    bool ownQObjects_;
    ObjectMethodMap objMethods_;
    ObjectReferenceMap objRefs_;
    LuaCallbackDispatcher dispatcher_; //signal->dispatcher_->Lua callback
};

template < typename T >
T GetValue( const LuaContext& lc, const QString& name ) {
    lua_getglobal( lc.LuaState(), name.toAscii().constData() );
    return luaL_checknumber( lc.LuaState(), -1 );
}

template < typename T >
QList< T > GetValues( const LuaContext& lc, const QString& name ) {
    if( !lua_istable( lc.LuaState(), -1 ) ) throw std::runtime_error( "Not a lua table" );
    return ParseLuaTableAsNumberList< T >( lc.LuaState(), -1 );
}

template <>
inline QString GetValue< QString >( const LuaContext& lc, const QString& name ) {
    lua_getglobal( lc.LuaState(), name.toAscii().constData() );
    return luaL_checkstring( lc.LuaState(), -1 );
}
template <>
inline QVariantMap GetValue< QVariantMap >( const LuaContext& lc, const QString& name ) {
    lua_getglobal( lc.LuaState(), name.toAscii().constData() );
    if( !lua_istable( lc.LuaState(), -1 ) ) throw std::runtime_error( "Not a lua table" );
    return ParseLuaTable( lc.LuaState(), -1 );
}
template <>
inline QVariantList GetValue< QVariantList >( const LuaContext& lc, const QString& name ) {
    lua_getglobal( lc.LuaState(), name.toAscii().constData() );
    if( !lua_istable( lc.LuaState(), -1 ) ) throw std::runtime_error( "Not a lua table" );
    return ParseLuaTableAsVariantList( lc.LuaState(), -1 );
}
template <>
inline QStringList GetValue< QStringList >( const LuaContext& lc, const QString& name ) {
    lua_getglobal( lc.LuaState(), name.toAscii().constData() );
    if( !lua_istable( lc.LuaState(), -1 ) ) throw std::runtime_error( "Not a lua table" );
    return ParseLuaTableAsStringList( lc.LuaState(), -1 );
}

}
