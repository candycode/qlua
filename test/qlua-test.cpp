#include <cassert>
#include <QPointer>
#include <iostream>
#include "../LuaContext.h"

#include "TestObject.h"

//------------------------------------------------------------------------------
int main() {
    try {

        TestObject* myobj2 = new TestObject;
        QPointer< TestObject > pMyObject2 = myobj2;
        {
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

        myobj2->setObjectName( "MyObject2" );
        ctx.AddQObject( myobj2, "myobj2", false, qlua::LuaContext::QOBJ_IMMEDIATE_DELETE );
        ctx.Eval( "print( 'object 2 name: '..myobj2.objectName )" );
        
        TestObject myobj3;
        ctx.AddQObject( &myobj3, "myobj3", false, qlua::LuaContext::QOBJ_NO_DELETE );
        ctx.Eval( "print( myobj3.copyString( 'hi' ) );"
                  "vm = myobj3.copyVariantMap( {key1=1,key2='hello'} );" 
                  "print( vm['key1'] .. ' ' .. vm['key2'] );"
                  "print( myobj3.createObject().objectName );" );

        ctx.Eval( "fl = myobj3.copyShortList( {1,2,3} );\n" 
                  "print( fl[1] .. ' ' .. fl[ 3 ] );\n" );
        }
        ///@warning works with Lua 5.2; luajit 2.0 beta9  won't invoke __gc!
        if( pMyObject2.isNull() ) std::cout << "Object 2 deleted by Lua" << std::endl;
        else std::cerr << "Object 2 not deleted!" << std::endl;         
         
    } catch( const std::exception& e ) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
