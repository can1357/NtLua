#pragma once
#include "state.hpp"

// Converts a C function into a Lua function assuming basic types.
//
struct native_function
{
    static constexpr const char* export_name = "native_function";
    const void* address = nullptr;

    // Allocator.
    //
    static native_function* push( lua_State* L );
    
    // Constructor.
    //
    static int create( lua_State* L );

    // Getter.
    //
    static native_function* check( lua_State* L, int index );

    // Member functions.
    //
    static int get_address( lua_State* L );
    static int invoke( lua_State* L );
    static int to_string( lua_State* L );

    // Registrar.
    //
    static void declare( lua_State* L )
    {
        static const luaL_Reg method_table[] = {
            { "new", create },
            { "address", get_address },
            { nullptr, nullptr }
        };

        static const luaL_Reg metatable[] = {
            { "__tostring", to_string },
            { "__call", invoke },
            { nullptr, nullptr }
        };

        luaL_openlib( L, export_name, method_table, 0 );
        luaL_newmetatable( L, export_name );

        luaL_openlib( L, 0, metatable, 0 );
        lua_pushliteral( L, "__index" );
        lua_pushvalue( L, -3 );
        lua_rawset( L, -3 );
        lua_pushliteral( L, "__metatable" );
        lua_pushvalue( L, -3 );
        lua_rawset( L, -3 );
        lua_pop( L, 2 );
    }
};