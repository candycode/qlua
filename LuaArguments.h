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

/// @file
/// @brief Declarations and definitions of constructors for creating C++ values
/// from Lua values.

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QString>
#include <QList>
#include <QGenericArgument>
#include <QGenericReturnArgument>
#include <QVector>

#include "LuaQtTypes.h"

/// QLua namespace
namespace qlua {

//------------------------------------------------------------------------------
/// @brief Interface for constructor objects which generate C++ values from
/// Lua values read from the Lua stack. 
///
/// There shall be exactly one and only one constructor per C++ type.
/// The QLua run-time (indirectly) invokes the ArgConstructor::Create() 
/// method whenever the invocation of a method of a QObject derived 
/// class instance is requested from Lua code. 
struct ArgConstructor {
    /// Create a QGenericArgument from Lua values on the Lua stack.
    virtual QGenericArgument Create( lua_State*, int ) const = 0;
    /// Virtual destructor.
    virtual ~ArgConstructor() {}
    /// Create a new instance of the current class.
    virtual ArgConstructor* Clone() const = 0;

};
/// ArgConstructor implementation for @c integer type.
class IntArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c integer value from Lua stack to data member then create 
    /// QGenericArgument referencing the data member.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( lua_State* L, int idx ) const {
        i_ = luaL_checkint( L, idx );
        return Q_ARG( int, i_ );
    }
    /// Make copy through copy constructor.
    IntArgConstructor* Clone() const {
        return new IntArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable int i_;
};
/// ArgConstructor implementation for @c float type.
class FloatArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c float value from Lua stack to data member then create 
    /// QGenericArgument referencing the data member. The value is converted
    /// from a Lua double precision number.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( lua_State* L, int idx ) const {
        f_ = float( luaL_checknumber( L, idx ) );
        return Q_ARG( double, f_ );
    }
    /// Make copy through copy constructor.
    FloatArgConstructor* Clone() const {
        return new FloatArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable float f_;
};
/// ArgConstructor implementation for @c double type.
class DoubleArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c double value from Lua stack to data member then create 
    /// QGenericArgument referencing the data member.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( lua_State* L, int idx ) const {
        d_ = luaL_checknumber( L, idx );
        return Q_ARG( double, d_ );
    }
    /// Make copy through copy constructor.
    DoubleArgConstructor* Clone() const {
        return new DoubleArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable double d_;
};
/// ArgConstructor implementation for @c QString type.
class StringArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c string value from Lua stack to data member then create 
    /// QGenericArgument referencing the data member.
    /// The value is converted from a Lua string type in plain ASCII format.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( lua_State* L, int idx ) const {
        s_ = luaL_checkstring( L, idx ); 
        return Q_ARG( QString, s_ );
    }
    /// Make copy through copy constructor.
    StringArgConstructor* Clone() const {
        return new StringArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QString s_;
};
/// ArgConstructor implementation for @c QVariantMap type.
class VariantMapArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c Lua value in table format from Lua stack to QVariantMap
    /// data member then create QGenericArgument referencing the data member.
    ///
    /// The value is converted by (possibly) recursively calling the 
    /// @c ParseLuaTable function.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( lua_State* L, int idx ) const {
        vm_ = ParseLuaTable( L, idx );
        return Q_ARG( QVariantMap, vm_ );
    }
    /// Make copy through copy constructor.
    VariantMapArgConstructor* Clone() const {
        return new VariantMapArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QVariantMap vm_;
};
/// ArgConstructor implementation for @c QVariantList type.
class VariantListArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c Lua value in table format from Lua stack to QVariantList
    /// data member then create QGenericArgument referencing the data member.
    ///
    /// The value is converted by recursively calling the 
    /// @c ParseLuaTableAsVariantList function.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( lua_State* L, int idx ) const {
        vl_ = ParseLuaTableAsVariantList( L, idx );
        return Q_ARG( QVariantList, vl_ );
    }
    /// Make copy through copy constructor.    
    VariantListArgConstructor* Clone() const {
        return new VariantListArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QVariantList vl_;
};
// ArgConstructor implementation for @c QObject* type.
class ObjectStarArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c Lua value in table format from Lua stack to QObject*
    /// data member then create QGenericArgument referencing the data member.
    ///
    /// The value is converted from either a Lua table wrapping a QObject* or
    /// directly from a QObject pointer.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( lua_State* L, int idx ) const {
        if( lua_istable( L, idx ) ) {
            lua_pushstring( L, "qobject__" );
            lua_gettable( L, idx );
            obj_ = *reinterpret_cast< QObject** >( lua_touserdata( L, -1 ) );
            lua_pop( L, 1 );
        } else if( lua_islightuserdata( L, idx ) ) {
            obj_ = reinterpret_cast< QObject* >( lua_touserdata( L, -1 ) );
        }
        return Q_ARG( QObject*, obj_ );
    }
    /// Make copy through copy constructor.    
    ObjectStarArgConstructor* Clone() const {
        return new ObjectStarArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QObject* obj_;
};
// ArgConstructor implementation for @c QWidget* type.
class WidgetStarArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c Lua value in table format from Lua stack to QWidget*
    /// data member then create QGenericArgument referencing the data member.
    ///
    /// The value is converted from either a Lua table wrapping a QWidget* or
    /// directly from a QWidget pointer.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( lua_State* L, int idx ) const {
        if( lua_istable( L, idx ) ) {
            lua_pushstring( L, "qobject__" );
            lua_gettable( L, idx );
            w_ = *reinterpret_cast< QWidget** >( lua_touserdata( L, -1 ) );
            lua_pop( L, 1 );
        } else if( lua_islightuserdata( L, idx ) ) {
            w_ = reinterpret_cast< QWidget* >( lua_touserdata( L, -1 ) );
        }
        return Q_ARG( QWidget*, w_ );
    }
    /// Make copy through copy constructor.    
    WidgetStarArgConstructor* Clone() const {
        return new WidgetStarArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QWidget* w_;
};
// ArgConstructor implementation for @c void* type.
class VoidStarArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c Lua value in table format from Lua stack to void*
    /// data member then create QGenericArgument referencing the data member.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance   
    QGenericArgument Create( lua_State* L, int idx ) const {
        v_ = const_cast< void* >( lua_topointer( L, idx ) );
        return Q_ARG( void*, v_ );
    }
    /// Make copy through copy constructor.    
    VoidStarArgConstructor* Clone() const {
        return new VoidStarArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable void* v_;
};
// ArgConstructor implementation for @c QList<T> type.
template< typename T >
class ListArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c Lua value in table format from Lua stack to QList<T>
    /// data member then create QGenericArgument referencing the data member.
    ///
    /// The element type can be any of:
    ///   - @c int
    ///   - @c short
    ///   - @c float
    ///   - @c double  
    /// A QList is generated by iterating over the table's values and converting
    /// each element to the requested numeric type.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( lua_State* L, int idx ) const {
        l_ = ParseLuaTableAsNumberList< T >( L, idx );
        return Q_ARG( QList< T >, l_ );
    }
    /// Make copy through copy constructor.    
    ListArgConstructor* Clone() const {
        return new ListArgConstructor< T >( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QList< T > l_;
};
// ArgConstructor implementation for @c QVector<T> type.
template< typename T >
class VectorArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c Lua value in table format from Lua stack to QVector<T>
    /// data member then create QGenericArgument referencing the data member.
    ///
    /// The element type can be any of:
    ///   - @c int
    ///   - @c short
    ///   - @c float
    ///   - @c double  
    /// A QVector is generated by iterating over the table's values and converting
    /// each element to the requested numeric type.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( lua_State* L, int idx ) const {
        v_ = ParseLuaTableAsNumberVector< T >( L, idx );
        return Q_ARG( QVector< T >, v_ );
    }
    /// Make copy through copy constructor.    
    VectorArgConstructor* Clone() const {
        return new VectorArgConstructor< T >( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QVector< T > v_;
};
// ArgConstructor implementation for @c QStringList type.
class StringListArgConstructor : public ArgConstructor {
public:
    /// @brief Copy @c Lua value in table format from Lua stack to QStringList
    /// data member then create QGenericArgument referencing the data member.
    /// @param L pointer to Lua stack
    /// @param idx position of value on the Lua stack
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( lua_State* L, int idx ) const {
        l_ = ParseLuaTableAsStringList( L, idx );
        return Q_ARG( QStringList, l_ );
    }
    /// Make copy through copy constructor.    
    StringListArgConstructor* Clone() const {
        return new StringListArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QStringList l_;
};
//------------------------------------------------------------------------------
class ReturnConstructor {
public:
    virtual void Push( lua_State* ) const = 0;
    virtual void Push( lua_State* , void* ) const = 0;
    virtual ~ReturnConstructor() {}
    virtual ReturnConstructor* Clone() const = 0;
    virtual QMetaType::Type Type() const = 0;
    QGenericReturnArgument Argument() const { return ga_; }
    template < typename T > void SetArg( T& arg ) {
        ga_ = QReturnArgument< T >( QMetaType::typeName( Type() ), arg );
    }
protected:
    QGenericReturnArgument ga_; 
};
class IntReturnConstructor : public ReturnConstructor {
public:
    IntReturnConstructor() {
        SetArg( i_ );
    }
    IntReturnConstructor( const IntReturnConstructor& other ) : i_( other.i_ ) {
        SetArg( i_ );
    }
    void Push( lua_State* L ) const {
        lua_pushinteger( L, i_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushinteger( L, *reinterpret_cast< int* >( value ) );
    }
    IntReturnConstructor* Clone() const {
        return new IntReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Int; }
private:
    int i_; 
};
class DoubleReturnConstructor : public ReturnConstructor {
public:
    DoubleReturnConstructor() {
       SetArg( d_ ); 
    }
    DoubleReturnConstructor( const DoubleReturnConstructor& other ) : d_( other.d_ ) {
       SetArg( d_ );
    }
    void Push( lua_State* L ) const {
        lua_pushnumber( L, d_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushnumber( L, *reinterpret_cast< double* >( value ) );
    }
    DoubleReturnConstructor* Clone() const {
        return new DoubleReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Double; }
private:
    double d_;
};
class FloatReturnConstructor : public ReturnConstructor {
public:
    FloatReturnConstructor() {
       SetArg( f_ ); 
    }
    FloatReturnConstructor( const FloatReturnConstructor& other ) : f_( other.f_ ) {
       SetArg( f_ );
    }
    void Push( lua_State* L ) const {
        lua_pushnumber( L, f_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushnumber( L, *reinterpret_cast< float* >( value ) );
    }
    FloatReturnConstructor* Clone() const {
        return new FloatReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Float; }
private:
    float f_;
};
class StringReturnConstructor : public ReturnConstructor {
public:
    StringReturnConstructor() {
        SetArg( s_ );
    }
    StringReturnConstructor( const StringReturnConstructor& other ) : s_( other.s_ ) {
        SetArg( s_ );
    }
    void Push( lua_State* L ) const {
        lua_pushstring( L, s_.toAscii().constData() );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushstring( L, reinterpret_cast< QString* >( value )->toAscii().constData() );
    }
    StringReturnConstructor* Clone() const {
        return new StringReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::QString; }
private:
    QString s_;
};
class VoidReturnConstructor : public ReturnConstructor {
public:
    void Push( lua_State*  ) const {}
    void Push( lua_State*, void* ) const {}
    VoidReturnConstructor* Clone() const {
        return new VoidReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Void; }
};
class VariantMapReturnConstructor : public ReturnConstructor {
public:
    VariantMapReturnConstructor() {
        SetArg( vm_ );   
    }
    VariantMapReturnConstructor( const VariantMapReturnConstructor& other ) : vm_( other.vm_ ) {
        SetArg( vm_ );
    }
    void Push( lua_State* L ) const {
        VariantMapToLuaTable( vm_, L );
    }
    void Push( lua_State* L, void* value ) const {
        VariantMapToLuaTable( *reinterpret_cast< QVariantMap* >( value ), L );
    }
    VariantMapReturnConstructor* Clone() const {
        return new VariantMapReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::QVariantMap; }
private:
    QVariantMap vm_;
};
class VariantListReturnConstructor : public ReturnConstructor {
public:
    VariantListReturnConstructor() {
        SetArg( vl_ );   
    }
    VariantListReturnConstructor( const VariantListReturnConstructor& other ) : vl_( other.vl_ ) {
        SetArg( vl_ );
    }
    void Push( lua_State* L ) const {
        VariantListToLuaTable( vl_, L );
    }
    void Push( lua_State* L, void* value ) const {
        VariantListToLuaTable( *reinterpret_cast< QVariantList* >( value ), L );
    }
    VariantListReturnConstructor* Clone() const {
        return new VariantListReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::QVariantList; }
private:
    QVariantList vl_;
};
class ObjectStarReturnConstructor : public ReturnConstructor {
public:
    ObjectStarReturnConstructor() {
        SetArg( obj_ );   
    }
    ObjectStarReturnConstructor( const ObjectStarReturnConstructor& other ) : obj_( other.obj_ ) {
        SetArg( obj_ );
    }
    void Push( lua_State* L ) const {
        lua_pushlightuserdata( L, obj_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushlightuserdata( L,  value );
    }
    ObjectStarReturnConstructor* Clone() const {
        return new ObjectStarReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::QObjectStar; }
private:
    QObject* obj_;
};
class WidgetStarReturnConstructor : public ReturnConstructor {
public:
    WidgetStarReturnConstructor() {
        SetArg( w_ );   
    }
    WidgetStarReturnConstructor( const WidgetStarReturnConstructor& other ) : w_( other.w_ ) {
        SetArg( w_ );
    }
    void Push( lua_State* L ) const {
        lua_pushlightuserdata( L, w_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushlightuserdata( L,  value );
    }
    WidgetStarReturnConstructor* Clone() const {
        return new WidgetStarReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::QWidgetStar; }
private:
    QWidget* w_;
};
class VoidStarReturnConstructor : public ReturnConstructor {
public:
    VoidStarReturnConstructor() {
        SetArg( v_ );   
    }
    VoidStarReturnConstructor( const VoidStarReturnConstructor& other ) : v_( other.v_ ) {
        SetArg( v_ );
    }
    void Push( lua_State* L ) const {
        lua_pushlightuserdata( L, v_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushlightuserdata( L,  value );
    }
    VoidStarReturnConstructor* Clone() const {
        return new VoidStarReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::VoidStar; }
private:
    void* v_;
};

template < typename T >
class ListReturnConstructor : public ReturnConstructor {
public:
    ListReturnConstructor() {
        SetArg( l_ );   
    }
    ListReturnConstructor( const ListReturnConstructor& other ) : l_( other.l_ ) {
        SetArg( l_ );
    }
    void Push( lua_State* L ) const {
        NumberListToLuaTable< T >( l_, L );
    }
    void Push( lua_State* L, void* value ) const {
        NumberListToLuaTable< T >( *reinterpret_cast< QList< T >* >( value ), L );
    }
    ListReturnConstructor* Clone() const {
        return new ListReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Type( QMetaType::type( TypeName< QList< T > >() ) ); }
private:
    QList< T > l_;
};

template < typename T >
class VectorReturnConstructor : public ReturnConstructor {
public:
    VectorReturnConstructor() {
        SetArg( v_ );   
    }
    VectorReturnConstructor( const VectorReturnConstructor& other ) : v_( other.v_ ) {
        SetArg( v_ );
    }
    void Push( lua_State* L ) const {
        NumberVectorToLuaTable< T >( v_, L );
    }
    void Push( lua_State* L, void* value ) const {
        NumberVectorToLuaTable< T >( *reinterpret_cast< QVector< T >* >( value ), L );
    }
    VectorReturnConstructor* Clone() const {
        return new VectorReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Type( QMetaType::type( TypeName< QVector< T > >() ) ); }
private:
    QVector< T > v_;
};
class StringListReturnConstructor : public ReturnConstructor {
public:
    StringListReturnConstructor() {
        SetArg( l_ );   
    }
    StringListReturnConstructor( const StringListReturnConstructor& other ) : l_( other.l_ ) {
        SetArg( l_ );
    }
    void Push( lua_State* L ) const {
        StringListToLuaTable( l_, L );
    }
    void Push( lua_State* L, void* value ) const {
        StringListToLuaTable( *reinterpret_cast< QStringList* >( value ), L );
    }
    StringListReturnConstructor* Clone() const {
        return new StringListReturnConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Type( QMetaType::type( TypeName< QList< QString > >() ) ); }
private:
    QStringList l_;
};

//------------------------------------------------------------------------------
class ParameterWrapper {
public:
    ParameterWrapper() : ac_( 0 ) {}
    ParameterWrapper( const ParameterWrapper& other ) : ac_( 0 ) {
        if( other.ac_ ) ac_ = other.ac_->Clone();
    }
    ParameterWrapper( const QString& type ) : ac_( 0 ) {
        if( type == QMetaType::typeName( QMetaType::Int ) ) {
            ac_ = new IntArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::Double ) ) {
            ac_ = new DoubleArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::Float ) ) {
            ac_ = new FloatArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QString ) ) {
            ac_ = new StringArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QVariantMap ) ) {
            ac_ = new VariantMapArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QVariantList ) ) {
            ac_ = new VariantListArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QObjectStar ) ) {
            ac_ = new ObjectStarArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QStringList ) ) {
            ac_ = new StringListArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QWidgetStar ) ) {
            ac_ = new WidgetStarArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::VoidStar ) ) {
            ac_ = new VoidStarArgConstructor;
        } else if( type == QLUA_LIST_FLOAT64 ) {
            ac_ = new ListArgConstructor< double >;
        } else if( type == QLUA_LIST_FLOAT32 ) {
            ac_ = new ListArgConstructor< float >;
        } else if( type == QLUA_LIST_INT ) {
            ac_ = new ListArgConstructor< int >;
        } else if( type == QLUA_LIST_SHORT ) {
            ac_ = new ListArgConstructor< short >;
        } else if( type == QLUA_VECTOR_FLOAT64 ) {
            ac_ = new VectorArgConstructor< double >;
        } else if( type == QLUA_VECTOR_FLOAT32 ) {
            ac_ = new VectorArgConstructor< float >;
        } else if( type == QLUA_VECTOR_INT ) {
            ac_ = new VectorArgConstructor< int >;
        } else if( type == QLUA_VECTOR_SHORT ) {
            ac_ = new VectorArgConstructor< short >;
        } else throw std::logic_error( ( "Type " + type + " unknown" ).toStdString() );
    }
    QGenericArgument Arg( lua_State* L, int idx ) const {
        return ac_ ? ac_->Create( L, idx ) : QGenericArgument();
    }
    ~ParameterWrapper() { delete ac_; }
private:
    ArgConstructor* ac_;    
};

class ReturnWrapper {
public:
    ReturnWrapper() : rc_( 0 ) {}
    ReturnWrapper( const ReturnWrapper& other ) : rc_( 0 ), type_( other.type_ ) {
        if( other.rc_ ) rc_ = other.rc_->Clone();
    }
    ReturnWrapper( const QString& type ) : rc_( 0 ), type_( type ) {
        if( type_ == QMetaType::typeName( QMetaType::Int ) ) {
            rc_ = new IntReturnConstructor;
        } else if( type == QMetaType::typeName( QMetaType::Double ) ) {
            rc_ = new DoubleReturnConstructor;
        } else if( type == QMetaType::typeName( QMetaType::Float ) ) {
            rc_ = new FloatReturnConstructor;
        } else if( type_ == QMetaType::typeName( QMetaType::QString ) ) {
            rc_ = new StringReturnConstructor;
        } else if( type_ == QMetaType::typeName( QMetaType::QVariantMap ) ) {
            rc_ = new VariantMapReturnConstructor;
        } else if( type_ == QMetaType::typeName( QMetaType::QVariantList ) ) {
            rc_ = new VariantListReturnConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QObjectStar ) ) {
            rc_ = new ObjectStarReturnConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QStringList ) ) {
            rc_ = new StringListReturnConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QWidgetStar ) ) {
            rc_ = new WidgetStarReturnConstructor;
        } else if( type == QMetaType::typeName( QMetaType::VoidStar ) ) {
            rc_ = new VoidStarReturnConstructor;
        } else if( type == QLUA_LIST_FLOAT64 ) {
            rc_ = new ListReturnConstructor< double >;
        } else if( type == QLUA_LIST_FLOAT32 ) {
            rc_ = new ListReturnConstructor< float >;
        } else if( type == QLUA_LIST_INT ) {
            rc_ = new ListReturnConstructor< int >;
        } else if( type == QLUA_LIST_SHORT ) {
            rc_ = new ListReturnConstructor< short >;
        } else if( type == QLUA_VECTOR_FLOAT64 ) {
            rc_ = new VectorReturnConstructor< double >;
        } else if( type == QLUA_VECTOR_FLOAT32 ) {
            rc_ = new VectorReturnConstructor< float >;
        } else if( type == QLUA_VECTOR_INT ) {
            rc_ = new VectorReturnConstructor< int >;
        } else if( type == QLUA_VECTOR_SHORT ) {
            rc_ = new VectorReturnConstructor< short >;
        } else if( type_.isEmpty() ) rc_ = new VoidReturnConstructor;
        else throw std::logic_error( ( "Type " + type + " unknown" ).toStdString() );
    }
    void Push( lua_State* L ) const {
        rc_->Push( L );
    }
    void Push( lua_State* L, void* value ) const {
        rc_->Push( L, value ); // called from the callback dispatcher method
    }
    QGenericReturnArgument Arg() const { return rc_->Argument(); }
    const QString& Type() const { 
        return type_;
    }
    QMetaType::Type MetaType() const { return rc_->Type(); }
    ~ReturnWrapper() { delete rc_; }
private:
    ReturnConstructor* rc_;
    QString type_;
};

typedef QList< ParameterWrapper > ParamWrappers;
typedef QList< QByteArray > ParamTypes;
inline ParamWrappers GenerateParameterWrappers( const ParamTypes& pt ) {
    ParamWrappers pw;
    for( ParamTypes::const_iterator i = pt.begin(); i != pt.end(); ++i ) {
        pw.push_back( ParameterWrapper( *i ) );
    }
    return pw;
}

inline ReturnWrapper GenerateReturnWrapper( const QString& typeName ) {
    return ReturnWrapper( typeName );
}

}
