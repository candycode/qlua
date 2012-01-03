#pragma once

#include <QMap>
#include <QList>

#include "Arguments.h"


namespace qlua {

class LuaContext;

typedef QList< ReturnWrapper  > CBackParameterTypes;

void PushLuaValue( LuaContext* lc, QMetaType::Type type, void* arg );

//------------------------------------------------------------------------------
class LuaCBackMethod {
public:
    LuaCBackMethod( LuaContext* lc, const CBackParameterTypes& p, int luaCBackRef ) 
        : lc_( lc ), paramTypes_( p ), luaCBackRef_( luaCBackRef ) {}
    void Invoke( void **arguments );
    int CBackRef() const{ return luaCBackRef_; } 
private:
    LuaContext* lc_;
    CBackParameterTypes paramTypes_;
    int luaCBackRef_;
};


typedef int LuaCBackRef;
typedef int MethodId;

//------------------------------------------------------------------------------
class LuaCallbackDispatcher : public QObject {
public:
    LuaCallbackDispatcher( QObject* parent = 0 ) : QObject( parent ), lc_( 0 ) {}
    LuaCallbackDispatcher( LuaContext* lc ) : lc_( lc ) {}
    int qt_metacall( QMetaObject::Call c, int id, void **arguments ); 
    bool Connect( QObject *obj, 
                  int signalIdx,
                  const CBackParameterTypes& paramTypes,
                  int luaCBackRef );
    bool Disconnect( QObject *obj, 
                     int signalIdx,
                     int cbackStackIndex );
    void SetLuaContext( LuaContext* lc ) { lc_ = lc; };
    virtual ~LuaCallbackDispatcher() {
        for( QList< LuaCBackMethod* >::iterator i = luaCBackMethods_.begin();
             i != luaCBackMethods_.end(); ++i ) {
            delete *i;
        }
    }
private:
    LuaContext* lc_;
    QList< LuaCBackMethod* > luaCBackMethods_;
    QMap< LuaCBackRef, MethodId > cbackToMethodIndex_;
   
};
}
