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
#include <cassert>
#include <QPointer>
#include <iostream>
#include "../LuaContext.h"

#include "TestObject.h"

//------------------------------------------------------------------------------
int main() {
    try {

        qlua::LuaContext ctx;
        
        TestObject myobj;
        myobj.setObjectName( "MyObject" );
        //only add a single method to the Lua table
        ctx.AddQObject( &myobj, "myobj", false,
                        qlua::LuaContext::QOBJ_NO_DELETE, QStringList() << "emitSignal" );
        ctx.Eval( "qlua.connect( myobj, 'aSignal(QString)', "
                    "function(msg) print( 'Lua callback called with data: ' .. msg ); end );"
                  "print( 'object name: ' .. myobj.objectName );"
                  "qlua.connect( myobj, 'aSignal(QString)', myobj, 'aSlot(QString)' );"
                  "myobj.emitSignal('hello')" );
        
        TestObject* myobj2 = new TestObject;
        QPointer< TestObject > pMyObject2 = myobj2;
        myobj2->setObjectName( "MyObject2" );
        ctx.AddQObject( myobj2, "myobj2", false, qlua::LuaContext::QOBJ_IMMEDIATE_DELETE );
        ctx.Eval( "print( 'object 2 name: '..myobj2.objectName )" );
        ctx.Eval( "myobj2=nil;collectgarbage('collect')");
        if( pMyObject2.isNull() ) std::cout << "Object 2 garbage collected by Lua" << std::endl;
        else std::cerr << "Object 2 not garbage collected!" << std::endl;         
        
        TestObject myobj3;
        ctx.AddQObject( &myobj3, "myobj3", false, qlua::LuaContext::QOBJ_NO_DELETE );
        ctx.Eval( "print( myobj3.copyString( 'hi' ) );"
                  "vm = myobj3.copyVariantMap( {key1=1,key2='hello'} );" 
                  "print( vm['key1'] .. ' ' .. vm['key2'] );"
                  "print( myobj3.createObject().objectName );" );

        ctx.Eval( "fl = myobj3.copyShortList( {1,2,3} );\n" 
                  "print( fl[1] .. ' ' .. fl[ 3 ] );\n" );
         
    } catch( const std::exception& e ) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
