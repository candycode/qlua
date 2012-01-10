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
#include <QMap>
#include <QList>

#include "LuaArguments.h"


namespace qlua {

class LuaContext;

typedef QList< LArgWrapper  > CBackParameterTypes;

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
