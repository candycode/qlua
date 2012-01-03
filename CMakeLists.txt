cmake_minimum_required(VERSION 2.8)

project(qlua)

#Qt
find_package(Qt4)
include(${QT_USE_FILE})
#Lua
set( LUA_INCLUDE_DIR "/usr/local/lua/include" CACHE PATH "Lua include directory" )
set( LUA_LIB_DIR "/usr/local/lua/lib" CACHE PATH "Lua lib directory" )
set( LUA_LIBRARIES "lua" CACHE FILEPATH "Lua library" )

include_directories( ${LUA_INCLUDE_DIR} ${QT_INCLUDE_DIR} )
link_directories( ${LUA_LIB_DIR} ${QT_LIBRARY_DIR} )

#library
add_library( qlua LuaCallbackDispatcher.h   LuaContext.h   Arguments.h LuaQtTypes.h
                  LuaCallbackDispatcher.cpp LuaContext.cpp )
target_link_libraries( qlua ${LUA_LIBRARIES} )

#test app
set( MOC_HEADERS test/TestObject.h )
QT4_WRAP_CPP( MOC_SRCS ${MOC_HEADERS} )
add_executable( qluatest test/qlua-test.cpp ${MOC_SRCS} ${MOC_HEADERS} )
target_link_libraries( qluatest ${QT_LIBRARIES} ${LUA_LIBRARIES} qlua )
