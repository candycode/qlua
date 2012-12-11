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
///@brief Utility functions for converting data types between Lua and Qt.

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <iostream>
#include <string>

#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QGenericArgument>
#include <QList>
#include <QVector>

#define QLUA_LIST_FLOAT64 "QList<double>"
#define QLUA_LIST_FLOAT32 "QList<float>"
#define QLUA_LIST_INT "QList<int>"
#define QLUA_LIST_SHORT "QList<short>"
#define QLUA_VECTOR_FLOAT64 "QVector<double>"
#define QLUA_VECTOR_FLOAT32 "QVector<float>"
#define QLUA_VECTOR_INT "QVector<int>"
#define QLUA_VECTOR_SHORT "QVector<short>"
#define QLUA_STRING_LIST "QList<QString>"

/// QLua namespace
namespace qlua {

//------------------------------------------------------------------------------
template < typename T > const char* TypeName();

template <> inline const char* TypeName< QList< double > >() { return QLUA_LIST_FLOAT64; }
template <> inline const char* TypeName< QList< float > >() { return QLUA_LIST_FLOAT32; }
template <> inline const char* TypeName< QList< int > >() { return QLUA_LIST_INT; }
template <> inline const char* TypeName< QList< short > >() { return QLUA_LIST_SHORT; }
template <> inline const char* TypeName< QVector< double > >() { return QLUA_VECTOR_FLOAT64; }
template <> inline const char* TypeName< QVector< float > >() { return QLUA_VECTOR_FLOAT32; }
template <> inline const char* TypeName< QVector< int > >() { return QLUA_VECTOR_INT; }
template <> inline const char* TypeName< QVector< short > >() { return QLUA_VECTOR_SHORT; }
template <> inline const char* TypeName< QList< QString > >() { return QLUA_STRING_LIST; }

//------------------------------------------------------------------------------
inline
QString LuaKeyToQString( lua_State* L, int idx ) {
    if( lua_isnumber( L, idx ) ) {
        return QString( "%1" ).arg( lua_tointeger( L, idx ) );
    } else if( lua_isstring( L, idx ) ) {
        return lua_tostring( L, idx );
    } else return "";
}

//------------------------------------------------------------------------------
template < typename ToT, typename FromT >
bool ConvertibleTo( FromT v ) {
    return FromT( ToT( v ) ) == v;
}

//==============================================================================
// Lua -> C++

//------------------------------------------------------------------------------
/// Create QVariant from Lua value
/// @param L Lua state
/// @param idx index of value in Lua table
inline
QVariant LuaValueToQVariant( lua_State* L, int idx ) {
    if( lua_isboolean( L, idx ) ) {
        return bool( lua_toboolean( L, idx ) );
    } else if( lua_isnumber( L, idx ) ) {   
#ifdef QLUA_CONVERT_NUMBER
        const double N = lua_tonumber( L, idx );
        if( ConvertibleTo< int >( N ) ) return int( N );
        else if( ConvertibleTo< unsigned int >( N ) ) return (unsigned int)( N );
        else if( ConvertibleTo< long long >( N ) ) return long( N );
        else if( ConvertibleTo< unsigned long long >( N ) ) return (unsigned long long)( N );
        else if( ConvertibleTo< float >( N ) ) return float( N );
        else return N;
#else
        return lua_tonumber( L, idx );
#endif
    } else if( lua_islightuserdata( L, idx ) ) {
        return lua_topointer( L, idx ); 
    } else if( lua_isstring( L, idx ) ) {
        return lua_tostring( L, idx );
    } else return QVariant();
}

//------------------------------------------------------------------------------
/// @brief Create QList<T> from Lua table where T is @c int @c short @c float or @c double
///
/// @param L Lua State
/// @param stackTableIndex index of table in Lua stack
template < typename T >
QList< T > ParseLuaTableAsNumberList( lua_State* L, int stackTableIndex ) {
      luaL_checktype( L, stackTableIndex, LUA_TTABLE );
#if LUA_VERSION_NUM > 501 
    int tableSize = int( lua_rawlen( L, stackTableIndex ) );
#else
    int tableSize = int( lua_objlen( L, stackTableIndex ) );
#endif
    QList< T > list;
    list.reserve( int( tableSize ) );
    ++tableSize;
    for( int i = 1; i != tableSize; ++i ) {
        lua_rawgeti( L, stackTableIndex, i );
        list.push_back( T( lua_tonumber( L, -1 ) ) );
        lua_pop( L, 1 );
    }
    return list;
}
//------------------------------------------------------------------------------
/// @brief Create QStringList from Lua table.
///
/// @param L Lua state
/// @param stackTableIndex index of table in Lua stack
inline
QStringList ParseLuaTableAsStringList( lua_State* L, int stackTableIndex ) {
      luaL_checktype( L, stackTableIndex, LUA_TTABLE );
#if LUA_VERSION_NUM > 501 
    int tableSize = int( lua_rawlen( L, stackTableIndex ) );
#else
    int tableSize = int( lua_objlen( L, stackTableIndex ) );
#endif
    QStringList list;
    list.reserve( int( tableSize ) );
    ++tableSize;
    for( int i = 1; i != tableSize; ++i ) {
        lua_rawgeti( L, stackTableIndex, i );
        list.push_back( QString( lua_tostring( L, -1 ) ) );
        lua_pop( L, 1 );
    }
    return list;
}
//------------------------------------------------------------------------------
/// @brief QVector<T> from Lua table where T is @c int @c short @c float or @c double
///
/// @param L Lua State
/// @param stackTableIndex index of table in Lua stack
template < typename T >
QVector< T > ParseLuaTableAsNumberVector( lua_State* L, int stackTableIndex ) {
#if LUA_VERSION_NUM > 501 
    int tableSize = int( lua_rawlen( L, stackTableIndex ) );
#else
    int tableSize = int( lua_objlen( L, stackTableIndex ) );
#endif
    QVector< T > v;
    v.resize( int( tableSize ) );
    ++tableSize;
    typename QVector< T >::iterator vi = v.begin();
    for( int i = 1; i != tableSize; ++i, ++vi ) {
        lua_rawgeti( L, stackTableIndex, i );
        *vi = T( lua_tonumber( L, -1 ) );
        lua_pop( L, 1 );
    }
    return v;
}

//------------------------------------------------------------------------------
/// @brief Create QVariantMap from Lua table.
///
/// @param L Lua State
/// @param stackTableIndex index of table in Lua stack
/// @param removeTable if @ true table is removed from stack, this is useful
///        when revursively invoking the function to guarantee that after it
///        returns no table is left on the stack. 
inline
QVariantMap ParseLuaTable( lua_State* L, int stackTableIndex, bool removeTable = true ) {
    luaL_checktype( L, stackTableIndex, LUA_TTABLE );
    QVariantMap m;
    lua_pushnil(L);  // first key
    stackTableIndex = stackTableIndex < 0 ? stackTableIndex - 1 : stackTableIndex;
    while( lua_next( L, stackTableIndex ) != 0 ) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        QString key = LuaKeyToQString( L, -2 );
        QVariant value = lua_istable( L, -1 ) ? ParseLuaTable( L, -1, false ) : 
                         LuaValueToQVariant( L, -1 );
        m[ key ] = value;
        lua_pop(L, 1);
    }
    if( removeTable ) lua_pop( L, 1 ); // remvove table
    return m;
}
//------------------------------------------------------------------------------
/// @brief Create QVariantList from Lua table.
///
/// @param L Lua State
/// @param stackTableIndex index of table in Lua stack
inline
QVariantList ParseLuaTableAsVariantList( lua_State* L, int stackTableIndex ) {
    luaL_checktype( L, stackTableIndex, LUA_TTABLE );
    QVariantList l;
    lua_pushnil(L);  // first key
    stackTableIndex = stackTableIndex < 0 ? stackTableIndex - 1 : stackTableIndex;
    while( lua_next( L, stackTableIndex ) != 0 ) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        QVariant value = lua_istable( L, -1 ) ? ParseLuaTable( L, -1, false ) : 
                         LuaValueToQVariant( L, -1 );
        l.push_back( value );
        lua_pop(L, 1);
    }
    return l;
}


//=============================================================================
// C++ -> Lua

void VariantMapToLuaTable( const QVariantMap&, lua_State* );
void VariantListToLuaTable( const QVariantList&, lua_State* );

//------------------------------------------------------------------------------
/// @brief Create Lua value from QVariant and push it on the Lua stack.
///
/// @param v QVariant
/// @param L Lua state
inline
void VariantToLuaValue( const QVariant& v, lua_State* L ) {

    switch( v.type() ) {
        case QVariant::Map: VariantMapToLuaTable( v.toMap(), L );
                            break;
        case QVariant::List: VariantListToLuaTable( v.toList(), L );
                             break;
        case QVariant::String: lua_pushstring( L, v.toString().toAscii().constData() );
                               break; 
        case QVariant::Int: lua_pushinteger( L, v.toInt() );
                            break;
        case QVariant::UInt: lua_pushnumber( L, v.toUInt() );
                             break;
        case QVariant::LongLong: lua_pushnumber( L, v.toLongLong() );
                                 break;
        case QVariant::ULongLong: lua_pushnumber( L, v.toULongLong() );
                                  break;
        case QVariant::Bool: lua_pushboolean( L, v.toBool() );
                             break;
        case QVariant::Double: lua_pushnumber( L, v.toDouble() );
                               break;
        default: break;
    }
}

//------------------------------------------------------------------------------
/// @brief Create Lua table from QVariantMap and push it on the Lua stack.
///
/// @param vm QVariantMap
/// @param L Lua state
inline
void VariantMapToLuaTable( const QVariantMap& vm, lua_State* L ) {
    lua_newtable( L ); 
    for( QVariantMap::const_iterator i = vm.begin(); i != vm.end(); ++i ) {
        lua_pushstring( L, i.key().toAscii().constData() );
        VariantToLuaValue( i.value(), L );
        lua_rawset( L, -3 );
    }
}
//------------------------------------------------------------------------------
/// @brief Create Lua table from QVariantList and push it on the Lua stack.
///
/// @param vl QVariantList
/// @param L Lua state
inline
void VariantListToLuaTable( const QVariantList& vl, lua_State* L ) {
    lua_newtable( L ); 
    int i = 1;
    for( QVariantList::const_iterator v = vl.begin(); v != vl.end(); ++v, ++i ) {
        lua_pushinteger( L, i );
        VariantToLuaValue( *v, L );
        lua_rawset( L, -3 );
    }
}
//------------------------------------------------------------------------------
/// @brief Create Lua table from QList<T> where T is a number and push it on the Lua stack.
///
/// @param l QList
/// @param L Lua state
template < typename T >
void NumberListToLuaTable( const QList< T >& l, lua_State* L ) {
    lua_newtable( L ); 
    int i = 1;
    for( typename QList< T >::const_iterator v = l.begin(); v != l.end(); ++v, ++i ) {
        lua_pushnumber( L, *v );
        lua_rawseti( L, -2, i );
    }
}
//------------------------------------------------------------------------------
/// @brief Create Lua table from QVector<T> where T is a number and push it on the Lua stack.
///
/// @param v QVector
/// @param L Lua state
template < typename T >
void NumberVectorToLuaTable( const QVector< T >& v, lua_State* L ) {
    lua_newtable( L ); 
    int i = 1;
    for( typename QVector< T >::const_iterator vi = v.begin(); vi != v.end(); ++vi, ++i ) {
        lua_pushnumber( L, *vi );
        lua_rawseti( L, -2, i );
    }
}
//------------------------------------------------------------------------------
/// @brief Create Lua table from QStringList and push it on the Lua stack.
///
/// @param sl QStringList
/// @param L Lua state
inline
void StringListToLuaTable( const QStringList& sl, lua_State* L ) {
    lua_newtable( L ); 
    int i = 1;
    for( QStringList::const_iterator v = sl.begin(); v != sl.end(); ++v, ++i ) {
        lua_pushinteger( L, i );
        lua_pushstring( L, v->toAscii().constData() );
        lua_rawset( L, -3 );
    }
}
}
