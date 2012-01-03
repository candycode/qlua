

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
    int methodIdx = cbackToMethodIndex_.value( luaCBackRef, -1 );
    if( methodIdx < 0 ) {
        methodIdx = luaCBackMethods_.size();
        cbackToMethodIndex_[ luaCBackRef ] = methodIdx;
        luaCBackMethods_.push_back(
            new LuaCBackMethod( lc_, paramTypes, luaCBackRef ) );
    }
    return QMetaObject::connect( obj, signalIdx, this, methodIdx + metaObject()->methodCount() );
}
//------------------------------------------------------------------------------
// precondition: value referenced object (i.e. function must already be on the stack at index idx )
bool LuaCallbackDispatcher::Disconnect( QObject *obj, 
                                        int signalIdx,
                                        int cbackStackIndex ) {
    int m = 0;
    bool ok = true;
    for( QList< LuaCBackMethod* >::iterator i = luaCBackMethods_.begin();
          i != luaCBackMethods_.end(); ++i, ++m ) {
         lua_rawgeti( lc_->LuaState(), LUA_REGISTRYINDEX, ( *i )->CBackRef() );
         if( cbackStackIndex < 0 ) --cbackStackIndex;
#if LUA_VERSION_NUM > 501
         const bool same = lua_compare( lc_->LuaState(), cbackStackIndex, -1, LUA_OPEQ );
#else
         const bool same = lua_equal( lc_->LuaState(), cbackStackIndex, -1 );
#endif
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
    ++arguments; // first parameter is return argument!
    for( CBackParameterTypes::const_iterator i = paramTypes_.begin();
         i != paramTypes_.end(); ++i, ++arguments ) {
        i->Push( lc_->LuaState(), *arguments );
        //UV XXX temporary solution, need to ad LuaContext reference
        //into Wrappers
        if( i->Type() == QMetaType::typeName( QMetaType::QObjectStar ) ) {
            QObject* obj = reinterpret_cast< QObject* >( lua_touserdata( lc_->LuaState(), -1 ) );
            lua_pop( lc_->LuaState(), 1 );
            lc_->AddQObject( obj );
        }
    }
    lua_pcall( lc_->LuaState(), paramTypes_.size(), 0, 0 );
}

}


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
