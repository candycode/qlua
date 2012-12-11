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
/// The QLua run-time (indirectly) invokes the QArgConstructor::Create() 
/// method whenever the invocation of a method of a QObject derived 
/// class instance is requested from Lua code. 
struct QArgConstructor {
    /// Create a QGenericArgument from Lua values on the Lua stack.
    virtual QGenericArgument Create( lua_State*, int ) const = 0;
    /// Virtual destructor.
    virtual ~QArgConstructor() {}
    /// Create a new instance of the current class.
    virtual QArgConstructor* Clone() const = 0;

};
/// QArgConstructor implementation for @c integer type.
class IntQArgConstructor : public QArgConstructor {
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
    IntQArgConstructor* Clone() const {
        return new IntQArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable int i_;
};
/// QArgConstructor implementation for @c float type.
class FloatQArgConstructor : public QArgConstructor {
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
    FloatQArgConstructor* Clone() const {
        return new FloatQArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable float f_;
};
/// QArgConstructor implementation for @c double type.
class DoubleQArgConstructor : public QArgConstructor {
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
    DoubleQArgConstructor* Clone() const {
        return new DoubleQArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable double d_;
};
/// QArgConstructor implementation for @c QString type.
class StringQArgConstructor : public QArgConstructor {
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
    StringQArgConstructor* Clone() const {
        return new StringQArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QString s_;
};
/// QArgConstructor implementation for @c QVariantMap type.
class VariantMapQArgConstructor : public QArgConstructor {
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
    VariantMapQArgConstructor* Clone() const {
        return new VariantMapQArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QVariantMap vm_;
};
/// QArgConstructor implementation for @c QVariantList type.
class VariantListQArgConstructor : public QArgConstructor {
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
    VariantListQArgConstructor* Clone() const {
        return new VariantListQArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QVariantList vl_;
};
/// QArgConstructor implementation for @c QObject* type.
class ObjectStarQArgConstructor : public QArgConstructor {
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
    ObjectStarQArgConstructor* Clone() const {
        return new ObjectStarQArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QObject* obj_;
};
/// QArgConstructor implementation for @c QWidget* type.
class WidgetStarQArgConstructor : public QArgConstructor {
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
    WidgetStarQArgConstructor* Clone() const {
        return new WidgetStarQArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QWidget* w_;
};
/// QArgConstructor implementation for @c void* type.
class VoidStarQArgConstructor : public QArgConstructor {
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
    VoidStarQArgConstructor* Clone() const {
        return new VoidStarQArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable void* v_;
};
/// QArgConstructor implementation for @c QList<T> type.
template< typename T >
class ListQArgConstructor : public QArgConstructor {
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
    ListQArgConstructor* Clone() const {
        return new ListQArgConstructor< T >( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QList< T > l_;
};
/// QArgConstructor implementation for @c QVector<T> type.
template< typename T >
class VectorQArgConstructor : public QArgConstructor {
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
    VectorQArgConstructor* Clone() const {
        return new VectorQArgConstructor< T >( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QVector< T > v_;
};
/// QArgConstructor implementation for @c QStringList type.
class StringListQArgConstructor : public QArgConstructor {
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
    StringListQArgConstructor* Clone() const {
        return new StringListQArgConstructor( *this );
    }
private:
    /// Storage for value read from Lua stack.
    mutable QStringList l_;
};
//------------------------------------------------------------------------------
/// @brief Abstract base class for return constructors which create Lua values
/// from C++ values.
class LArgConstructor {
public:
    /// @brief Push value on Lua stack, invoked when a value must be returned
    /// from a method invoked from Lua.
    virtual void Push( lua_State* ) const = 0;
    /// @brief Push value read from a specific memory location on Lua stack,
    /// invoked when calling a method as result of signal emission.
    virtual void Push( lua_State* , void* ) const = 0;
    /// Virtual destructor.
    virtual ~LArgConstructor() {}
    /// Return copy of object.
    virtual LArgConstructor* Clone() const = 0;
    /// Return type of constructed data.
    virtual QMetaType::Type Type() const = 0;
    /// @brief Return QGenericReturnArguments holding a reference to the
    /// memory location where the returned value is stored.
    QGenericReturnArgument Argument() const { return ga_; }
    /// @brief Return @c true if type is a pointer to a QObject-derived object.
    ///
    /// This is required to have the QLua run-time add the passed QObject into
    /// the Lua context. The other option is to have LArgConstructors::Push
    /// receive a reference to a LuaContext which introduces a two-way
    /// dependency between LArgConstructor and LuaContext.
    virtual bool IsQObjectPtr() const { return false; }
protected:
    /// Creates return argument of the proper type.
    template < typename T > void SetArg( T& arg ) {
        ga_ = QReturnArgument< T >( QMetaType::typeName( Type() ), arg );
    }
    /// Placeholder for returned data. 
    QGenericReturnArgument ga_; // not private, breaks encapsulation
};
/// LArgConstructor implementation for @c integer type
class IntLArgConstructor : public LArgConstructor {
public:
    IntLArgConstructor() {
        SetArg( i_ );
    }
    IntLArgConstructor( const IntLArgConstructor& other ) : i_( other.i_ ) {
        SetArg( i_ );
    }
    void Push( lua_State* L ) const {
        lua_pushinteger( L, i_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushinteger( L, *reinterpret_cast< int* >( value ) );
    }
    IntLArgConstructor* Clone() const {
        return new IntLArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Int; }
private:
    int i_; 
};
/// LArgConstructor implementation for @c double type
class DoubleLArgConstructor : public LArgConstructor {
public:
    DoubleLArgConstructor() {
       SetArg( d_ ); 
    }
    DoubleLArgConstructor( const DoubleLArgConstructor& other ) : d_( other.d_ ) {
       SetArg( d_ );
    }
    void Push( lua_State* L ) const {
        lua_pushnumber( L, d_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushnumber( L, *reinterpret_cast< double* >( value ) );
    }
    DoubleLArgConstructor* Clone() const {
        return new DoubleLArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Double; }
private:
    double d_;
};
/// LArgConstructor implementation for @c float type
class FloatLArgConstructor : public LArgConstructor {
public:
    FloatLArgConstructor() {
       SetArg( f_ ); 
    }
    FloatLArgConstructor( const FloatLArgConstructor& other ) : f_( other.f_ ) {
       SetArg( f_ );
    }
    void Push( lua_State* L ) const {
        lua_pushnumber( L, f_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushnumber( L, *reinterpret_cast< float* >( value ) );
    }
    FloatLArgConstructor* Clone() const {
        return new FloatLArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Float; }
private:
    float f_;
};
/// LArgConstructor implementation for @c QString type
class StringLArgConstructor : public LArgConstructor {
public:
    StringLArgConstructor() {
        SetArg( s_ );
    }
    StringLArgConstructor( const StringLArgConstructor& other ) : s_( other.s_ ) {
        SetArg( s_ );
    }
    void Push( lua_State* L ) const {
        lua_pushstring( L, s_.toAscii().constData() );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushstring( L, reinterpret_cast< QString* >( value )->toAscii().constData() );
    }
    StringLArgConstructor* Clone() const {
        return new StringLArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::QString; }
private:
    QString s_;
};
/// LArgConstructor implementation for @c void type
class VoidLArgConstructor : public LArgConstructor {
public:
    void Push( lua_State*  ) const {}
    void Push( lua_State*, void* ) const {}
    VoidLArgConstructor* Clone() const {
        return new VoidLArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Void; }
};
/// LArgConstructor implementation for @c QVariantMap type
class VariantMapLArgConstructor : public LArgConstructor {
public:
    VariantMapLArgConstructor() {
        SetArg( vm_ );   
    }
    VariantMapLArgConstructor( const VariantMapLArgConstructor& other ) : vm_( other.vm_ ) {
        SetArg( vm_ );
    }
    void Push( lua_State* L ) const {
        VariantMapToLuaTable( vm_, L );
    }
    void Push( lua_State* L, void* value ) const {
        VariantMapToLuaTable( *reinterpret_cast< QVariantMap* >( value ), L );
    }
    VariantMapLArgConstructor* Clone() const {
        return new VariantMapLArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::QVariantMap; }
private:
    QVariantMap vm_;
};
/// LArgConstructor implementation for @c QVariantList type
class VariantListLArgConstructor : public LArgConstructor {
public:
    VariantListLArgConstructor() {
        SetArg( vl_ );   
    }
    VariantListLArgConstructor( const VariantListLArgConstructor& other ) : vl_( other.vl_ ) {
        SetArg( vl_ );
    }
    void Push( lua_State* L ) const {
        VariantListToLuaTable( vl_, L );
    }
    void Push( lua_State* L, void* value ) const {
        VariantListToLuaTable( *reinterpret_cast< QVariantList* >( value ), L );
    }
    VariantListLArgConstructor* Clone() const {
        return new VariantListLArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::QVariantList; }
private:
    QVariantList vl_;
};
/// LArgConstructor implementation for @c QObject* type
class ObjectStarLArgConstructor : public LArgConstructor {
public:
    ObjectStarLArgConstructor() {
        SetArg( obj_ );   
    }
    ObjectStarLArgConstructor( const ObjectStarLArgConstructor& other ) : obj_( other.obj_ ) {
        SetArg( obj_ );
    }
    void Push( lua_State* L ) const {
        lua_pushlightuserdata( L, obj_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushlightuserdata( L,  value );
    }
    ObjectStarLArgConstructor* Clone() const {
        return new ObjectStarLArgConstructor( *this );
    }
    bool IsQObjectPtr() const { return true; }
    QMetaType::Type Type() const { return QMetaType::QObjectStar; }
private:
    QObject* obj_;
};
/// LArgConstructor implementation for @c QWidget* type
class WidgetStarLArgConstructor : public LArgConstructor {
public:
    WidgetStarLArgConstructor() {
        SetArg( w_ );   
    }
    WidgetStarLArgConstructor( const WidgetStarLArgConstructor& other ) : w_( other.w_ ) {
        SetArg( w_ );
    }
    void Push( lua_State* L ) const {
        lua_pushlightuserdata( L, w_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushlightuserdata( L,  value );
    }
    WidgetStarLArgConstructor* Clone() const {
        return new WidgetStarLArgConstructor( *this );
    }
    bool IsQObjectPtr() const { return true; }
    QMetaType::Type Type() const { return QMetaType::QWidgetStar; }
private:
    QWidget* w_;
};
/// LArgConstructor implementation for @c void* type
class VoidStarLArgConstructor : public LArgConstructor {
public:
    VoidStarLArgConstructor() {
        SetArg( v_ );   
    }
    VoidStarLArgConstructor( const VoidStarLArgConstructor& other ) : v_( other.v_ ) {
        SetArg( v_ );
    }
    void Push( lua_State* L ) const {
        lua_pushlightuserdata( L, v_ );
    }
    void Push( lua_State* L, void* value ) const {
        lua_pushlightuserdata( L,  value );
    }
    VoidStarLArgConstructor* Clone() const {
        return new VoidStarLArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::VoidStar; }
private:
    void* v_;
};
/// @brief LArgConstructor implementation for @c QList<T> type.
///
/// @c T is a numeric type; supported types are:
/// - int
/// - short
/// - float
/// - double
template < typename T >
class ListLArgConstructor : public LArgConstructor {
public:
    ListLArgConstructor() {
        SetArg( l_ );   
    }
    ListLArgConstructor( const ListLArgConstructor& other ) : l_( other.l_ ) {
        SetArg( l_ );
    }
    void Push( lua_State* L ) const {
        NumberListToLuaTable< T >( l_, L );
    }
    void Push( lua_State* L, void* value ) const {
        NumberListToLuaTable< T >( *reinterpret_cast< QList< T >* >( value ), L );
    }
    ListLArgConstructor* Clone() const {
        return new ListLArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Type( QMetaType::type( TypeName< QList< T > >() ) ); }
private:
    QList< T > l_;
};
/// @brief LArgConstructor implementation for @c QVector<T> type.
///
/// @c T is a numeric type; supported types are:
///   - @c int
///   - @c short
///   - @c float
///   - @c double
template < typename T >
class VectorLArgConstructor : public LArgConstructor {
public:
    VectorLArgConstructor() {
        SetArg( v_ );   
    }
    VectorLArgConstructor( const VectorLArgConstructor& other ) : v_( other.v_ ) {
        SetArg( v_ );
    }
    void Push( lua_State* L ) const {
        NumberVectorToLuaTable< T >( v_, L );
    }
    void Push( lua_State* L, void* value ) const {
        NumberVectorToLuaTable< T >( *reinterpret_cast< QVector< T >* >( value ), L );
    }
    VectorLArgConstructor* Clone() const {
        return new VectorLArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Type( QMetaType::type( TypeName< QVector< T > >() ) ); }
private:
    QVector< T > v_;
};
/// LArgConstructor implementation for @c QStringList type
class StringListLArgConstructor : public LArgConstructor {
public:
    StringListLArgConstructor() {
        SetArg( l_ );   
    }
    StringListLArgConstructor( const StringListLArgConstructor& other ) : l_( other.l_ ) {
        SetArg( l_ );
    }
    void Push( lua_State* L ) const {
        StringListToLuaTable( l_, L );
    }
    void Push( lua_State* L, void* value ) const {
        StringListToLuaTable( *reinterpret_cast< QStringList* >( value ), L );
    }
    StringListLArgConstructor* Clone() const {
        return new StringListLArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Type( QMetaType::type( TypeName< QList< QString > >() ) ); }
private:
    QStringList l_;
};

//------------------------------------------------------------------------------
/// @brief Wrapper for parameters in a QObject method invocation.
///
/// Whenever a new QObject is added to the Lua context, the signature of each
/// method is translated to an index and a list of QArgWrapper objects 
/// stored inside a LuaContext instance.
/// At invocation time the proper method is invoked through a call to
/// @c QMetaMethod::invoke passing the arguments returned by the QArgWrapper::Arg
/// method invoked on each parameter in the argument list.
/// QArgWrapper stores an instance of QArgConstructor used to create a
/// QGenericArgument from values on the Lua stack.
class QArgWrapper {
public:
    /// @brief Default constructor.
    QArgWrapper() : ac_( 0 ) {}
    /// Copy constructor: Clone QArgConstructor instance.
    QArgWrapper( const QArgWrapper& other ) : ac_( 0 ) {
        if( other.ac_ ) ac_ = other.ac_->Clone();
    }
    /// @brief Construct instance from type name. Creates proper instance of
    /// inner QArgConstructor from type info.
    QArgWrapper( const QString& type ) : ac_( 0 ) {
        if( type == QMetaType::typeName( QMetaType::Int ) ) {
            ac_ = new IntQArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::Double ) ) {
            ac_ = new DoubleQArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::Float ) ) {
            ac_ = new FloatQArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QString ) ) {
            ac_ = new StringQArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QVariantMap ) ) {
            ac_ = new VariantMapQArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QVariantList ) ) {
            ac_ = new VariantListQArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QObjectStar ) ) {
            ac_ = new ObjectStarQArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QStringList ) ) {
            ac_ = new StringListQArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QWidgetStar ) ) {
            ac_ = new WidgetStarQArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::VoidStar ) ) {
            ac_ = new VoidStarQArgConstructor;
        } else if( type == QLUA_LIST_FLOAT64 ) {
            ac_ = new ListQArgConstructor< double >;
        } else if( type == QLUA_LIST_FLOAT32 ) {
            ac_ = new ListQArgConstructor< float >;
        } else if( type == QLUA_LIST_INT ) {
            ac_ = new ListQArgConstructor< int >;
        } else if( type == QLUA_LIST_SHORT ) {
            ac_ = new ListQArgConstructor< short >;
        } else if( type == QLUA_VECTOR_FLOAT64 ) {
            ac_ = new VectorQArgConstructor< double >;
        } else if( type == QLUA_VECTOR_FLOAT32 ) {
            ac_ = new VectorQArgConstructor< float >;
        } else if( type == QLUA_VECTOR_INT ) {
            ac_ = new VectorQArgConstructor< int >;
        } else if( type == QLUA_VECTOR_SHORT ) {
            ac_ = new VectorQArgConstructor< short >;
        } else throw std::logic_error( ( "Type " + type + " unknown" ).toStdString() );
    }
    /// @brief Return QGenericArgument instance created from values on the Lua stack.
    ///
    /// Internally it calls QArgConstructor::Create to generate QGenericArguments from
    /// Lua values.
    QGenericArgument Arg( lua_State* L, int idx ) const {
        return ac_ ? ac_->Create( L, idx ) : QGenericArgument();
    }
    /// @brief Destructor; delete QArgConstructor instance.
    ~QArgWrapper() { delete ac_; }
private:
    /// Instance of QArgConstructor created from type information at construction
    /// time.
    QArgConstructor* ac_;    
};

/// @brief Wrapper for objects returned from QObject method invocation or passes
/// to Lua callbacks in response to emitted signals.
///
/// This class translates C++ values to Lua values and is used to both return
/// values from method invocations and translate the parameters received from
/// a signal to Lua values whenever a Lua callback invocation is triggered by
/// an emitted signal.
class LArgWrapper {
public:
    ///@brief Default constructor.
    LArgWrapper() : ac_( 0 ) {}
    ///@brief Copy constructor: Clones the internal Return constructor instance.
    LArgWrapper( const LArgWrapper& other ) : ac_( 0 ), type_( other.type_ ) {
        if( other.ac_ ) ac_ = other.ac_->Clone();
    }
    ///@brief Create instance from type name.
    ///
    ///An instance of LArgConstructor is created from the passes type name.
    LArgWrapper( const QString& type ) : ac_( 0 ), type_( type ) {
        if( type_ == QMetaType::typeName( QMetaType::Int ) ) {
            ac_ = new IntLArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::Double ) ) {
            ac_ = new DoubleLArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::Float ) ) {
            ac_ = new FloatLArgConstructor;
        } else if( type_ == QMetaType::typeName( QMetaType::QString ) ) {
            ac_ = new StringLArgConstructor;
        } else if( type_ == QMetaType::typeName( QMetaType::QVariantMap ) ) {
            ac_ = new VariantMapLArgConstructor;
        } else if( type_ == QMetaType::typeName( QMetaType::QVariantList ) ) {
            ac_ = new VariantListLArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QObjectStar ) ) {
            ac_ = new ObjectStarLArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QStringList ) ) {
            ac_ = new StringListLArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::QWidgetStar ) ) {
            ac_ = new WidgetStarLArgConstructor;
        } else if( type == QMetaType::typeName( QMetaType::VoidStar ) ) {
            ac_ = new VoidStarLArgConstructor;
        } else if( type == QLUA_LIST_FLOAT64 ) {
            ac_ = new ListLArgConstructor< double >;
        } else if( type == QLUA_LIST_FLOAT32 ) {
            ac_ = new ListLArgConstructor< float >;
        } else if( type == QLUA_LIST_INT ) {
            ac_ = new ListLArgConstructor< int >;
        } else if( type == QLUA_LIST_SHORT ) {
            ac_ = new ListLArgConstructor< short >;
        } else if( type == QLUA_VECTOR_FLOAT64 ) {
            ac_ = new VectorLArgConstructor< double >;
        } else if( type == QLUA_VECTOR_FLOAT32 ) {
            ac_ = new VectorLArgConstructor< float >;
        } else if( type == QLUA_VECTOR_INT ) {
            ac_ = new VectorLArgConstructor< int >;
        } else if( type == QLUA_VECTOR_SHORT ) {
            ac_ = new VectorLArgConstructor< short >;
        } else if( type_.isEmpty() ) ac_ = new VoidLArgConstructor;
        else throw std::logic_error( ( "Type " + type + " unknown" ).toStdString() );
    }
    /// @brief Push values stored in the inner LArgConstructor instance on the
    /// Lua stack.
    ///
    /// This is the method invoked to return values from a QObject method invocation.
    void Push( lua_State* L ) const {
        ac_->Push( L );
    }
    /// @brief Push values stored in passed memory location on the Lua stack.
    ///
    /// This is the method invoked when a Lua callback is called through
    /// @c QObject::qt_metacall (e.g. through a triggered signal). When Lua 
    /// functions are called through @c qt_metacall the list of arguments is stored 
    /// inside an array of void pointers; each parameter must therefore be converted to 
    /// the proper C++ type first and then translated to a Lua values.
    /// @param L Lua state
    /// @param value memory location to read from
    void Push( lua_State* L, void* value ) const {
        ac_->Push( L, value ); // called from the callback dispatcher method
    }
    /// @brief Return the location where the return argument passed to a method
    /// invocation shall be stored.
    ///
    /// This method is invoked to provide QMetaMethod::invoke with the location
    /// where the return value will be stored, which is the storage space provided
    /// by the LArgConstructor instance stored in instances of this class.
    /// After the method invocation returns the value in the LArgConstructor instance
    /// is pushed on the Lua stack through a call to LArgConstructor::Push(lua_State*).
    QGenericReturnArgument Arg() const { return ac_->Argument(); }
    /// Type name.
    const QString& Type() const { 
        return type_;
    }
    /// Meta type.
    QMetaType::Type MetaType() const { return ac_->Type(); }
    /// Return true if wrapped type is QObject pointer.
    bool IsQObjectPtr() const { return ac_->IsQObjectPtr(); }
    /// Delete LArgConstructor instance.
    ~LArgWrapper() { delete ac_; }
private:
    /// LArgConstructor instance created at construction time.
    LArgConstructor* ac_;
    /// Qt type name of data stored in ac_.
    QString type_;
};

typedef QList< QArgWrapper > QArgWrappers;
typedef QList< QByteArray > ArgumentTypes;

/// @brief Generate QArgWrapper list from parameter type names as
/// returned by @c QMetaMethod::parameterTypes().
inline QArgWrappers GenerateQArgWrappers( const ArgumentTypes& at ) {
    QArgWrappers aw;
    for( ArgumentTypes::const_iterator i = at.begin(); i != at.end(); ++i ) {
        aw.push_back( QArgWrapper( *i ) );
    }
    return aw;
}
/// @brief Create LArgWrapper instance from type name.
inline LArgWrapper GenerateLArgWrapper( const QString& typeName ) {
    return LArgWrapper( typeName );
}

}
