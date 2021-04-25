#include "native_function.hpp"

// Allocator.
//
native_function* native_function::push( lua_State* L )
{
    native_function* fn = ( native_function* ) lua_newuserdata( L, sizeof( native_function ) );
    luaL_getmetatable( L, export_name );
    lua_setmetatable( L, -2 );
    return fn;
}

// Constructor.
//
int native_function::create( lua_State* L )
{
    push( L )->address = ( void* ) luaL_checkunsigned( L, 1 );
    return 1;
}

// Getter.
//
native_function* native_function::check( lua_State* L, int index )
{
    native_function* p;
    luaL_checktype( L, index, LUA_TUSERDATA );
    p = ( native_function* ) luaL_checkudata( L, index, export_name );
    if ( !p )
        luaL_error( L, "Type mismatch, expected [%s]\n", export_name );
    return p;
}

// Member functions.
//
int native_function::get_address( lua_State* L )
{
    lua_pushunsigned( L, ( uint64_t ) ( ( native_function* ) check( L, 1 ) )->address );
    return 1;
}
int native_function::invoke( lua_State* L )
{
    native_function* fn = ( native_function* ) check( L, 1 );

    // Get number of arguments.
    //
    int n = lua_gettop( L ) - 1;
    if ( n >= 32 )
        luaL_error( L, "Too many arguments provided %d vs maximum of 16\n", n );

    // Recursively create the call frame and call out.
    //
    auto rec = [ & ] <typename... Tx> ( auto&& self, const void* fn, size_t i, Tx... args ) -> uint64_t
    {
        if constexpr ( sizeof...( Tx ) <= 32 )
        {
            if ( i == n )
                return ( ( uint64_t( __stdcall* )( Tx... ) ) fn )( args... );
            else
                return self( self, fn, i + 1, args..., lua_asintrinsic( L, i + 2 ) );
        }
        __assume( 0 );
    };
    uint64_t result = rec( rec, fn->address, 0 );

    // Push the result and return.
    //
    lua_pushunsigned( L, result );
    return 1;
}
int native_function::to_string( lua_State* L )
{
    lua_pushfstring( L, "native_function (0x%p)", check( L, 1 )->address );
    return 1;
}