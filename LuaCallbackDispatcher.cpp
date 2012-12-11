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

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}


#include "LuaContext.h"
#include "LuaCallbackDispatcher.h"

namespace qlua {

//------------------------------------------------------------------------------
bool LuaCallbackDispatcher::Connect( QObject *obj, 
                                     int signalIdx,
                                     const CBackParameterTypes& paramTypes,
                                     LuaCBackRef luaCBackRef ) {
    // check if Lua function reference already stored in database;
    // if not create a new 'dynamic method' and map function reference
    // to the newly created method;
    // the index of a new method is the metod array size
    int methodIdx = cbackToMethodIndex_.value( luaCBackRef, -1 );
    if( methodIdx < 0 ) {
        methodIdx = luaCBackMethods_.size();
        cbackToMethodIndex_[ luaCBackRef ] = methodIdx;
        luaCBackMethods_.push_back(
            new LuaCBackMethod( lc_, paramTypes, luaCBackRef ) );
}
    // connect signal to method in method array
    return QMetaObject::connect( obj, signalIdx, this, methodIdx + metaObject()->methodCount() );
}
//------------------------------------------------------------------------------
// precondition: Lua function available in lua stack
bool LuaCallbackDispatcher::Disconnect( QObject *obj, 
                                        int signalIdx,
                                        int cbackStackIndex ) {
    if( !lua_isfunction( lc_->LuaState(), cbackStackIndex ) ) {
        RaiseLuaError( lc_->LuaState(), "No function to disconnect found" );
        return false;
	}
    int m = 0;
    bool ok = true;
    // iterate over callback methods, each method is associated with
    // one and only one Lua function
    for( QList< LuaCBackMethod* >::iterator i = luaCBackMethods_.begin();
          i != luaCBackMethods_.end(); ++i, ++m ) {
         // get lua function associated with lua reference
         lua_rawgeti( lc_->LuaState(), LUA_REGISTRYINDEX, ( *i )->CBackRef() );
         if( cbackStackIndex < 0 ) --cbackStackIndex;
         // compare function with Lua function to disconnect
#if LUA_VERSION_NUM > 501
         const bool same = lua_compare( lc_->LuaState(), cbackStackIndex, -1, LUA_OPEQ );
#else
         const bool same = lua_equal( lc_->LuaState(), cbackStackIndex, -1 );
#endif
         // if match disconnect signal from method and remove reference
         if( same ) {
             ok = ok && QMetaObject::disconnect( obj, signalIdx, this, m + metaObject()->methodCount() );
             luaL_unref( lc_->LuaState(), LUA_REGISTRYINDEX, lua_tointeger( lc_->LuaState(), -1 ) );
             lua_pop( lc_->LuaState(), 1 );
         } else lua_pop( lc_->LuaState(), 1 );
    }   
    return ok;
}
//------------------------------------------------------------------------------
int LuaCallbackDispatcher::qt_metacall( QMetaObject::Call invoke, MethodId methodIndex, void **arguments ) {
    methodIndex = QObject::qt_metacall( invoke, methodIndex, arguments );
    if( methodIndex < 0 || invoke != QMetaObject::InvokeMetaMethod ) return methodIndex;
    luaCBackMethods_[ methodIndex ]->Invoke( arguments );
    return -1;
}
//------------------------------------------------------------------------------
void LuaCBackMethod::Invoke( void **arguments ) {
    lua_rawgeti( lc_->LuaState(), LUA_REGISTRYINDEX, luaCBackRef_ ); 
    ++arguments; // first parameter is placeholder for return argument! - ignore
    //iterate over arguments and push values on Lua stack
    for( CBackParameterTypes::const_iterator i = paramTypes_.begin();
         i != paramTypes_.end(); ++i, ++arguments ) {
        i->Push( lc_->LuaState(), *arguments );
        if( i->IsQObjectPtr() ) {
            QObject* obj = reinterpret_cast< QObject* >( lua_touserdata( lc_->LuaState(), -1 ) );
            lua_pop( lc_->LuaState(), 1 );
            lc_->AddQObject( obj );
        }
    }
    //call Lua function
    lua_pcall( lc_->LuaState(), paramTypes_.size(), 0, 0 );
}

}

// Pre-defined types in Qt meta-type environment:
//enum Type {
//        // these are merged with QVariant
//        Void = 0, Bool = 1, Int = 2, UInt = 3, LongLong = 4, ULongLong = 5,
//        Double = 6, QChar = 7, QVariantMap = 8, QVariantList = 9,
//        QString = 10, QStringList = 11, QByteArray = 12,
//        QBitArray = 13, QDate = 14, QTime = 15, QDateTime = 16, QUrl = 17,
//        QLocale = 18, QRect = 19, QRectF = 20, QSize = 21, QSizeF = 22,
//        QLine = 23, QLineF = 24, QPoint = 25, QPointF = 26, QRegExp = 27,
//        QVariantHash = 28, QEasingCurve = 29, LastCoreType = QEasingCurve,
//
//        FirstGuiType = 63 /* QColorGroup */,
//#ifdef QT3_SUPPORT
//        QColorGroup = 63,
//#endif
//        QFont = 64, QPixmap = 65, QBrush = 66, QColor = 67, QPalette = 68,
//        QIcon = 69, QImage = 70, QPolygon = 71, QRegion = 72, QBitmap = 73,
//        QCursor = 74, QSizePolicy = 75, QKeySequence = 76, QPen = 77,
//        QTextLength = 78, QTextFormat = 79, QMatrix = 80, QTransform = 81,
//        QMatrix4x4 = 82, QVector2D = 83, QVector3D = 84, QVector4D = 85,
//        QQuaternion = 86,
//        LastGuiType = QQuaternion,
//
//        FirstCoreExtType = 128 /* VoidStar */,
//        VoidStar = 128, Long = 129, Short = 130, Char = 131, ULong = 132,
//        UShort = 133, UChar = 134, Float = 135, QObjectStar = 136, QWidgetStar = 137,
//        QVariant = 138,
//        LastCoreExtType = QVariant,
//
//// This logic must match the one in qglobal.h
//#if defined(QT_COORD_TYPE)
//        QReal = 0,
//#elif defined(QT_NO_FPU) || defined(QT_ARCH_ARM) || defined(QT_ARCH_WINDOWSCE) || defined(QT_ARCH_SYMBIAN)
//        QReal = Float,
//#else
//        QReal = Double,
//#endif
//
//        User = 256
