#include "native_function.hpp"

// Helper function to invoke any function pointer.
//
template<size_t N = 16, typename... T>
static uint64_t call_any( void* fn, uint64_t* args, size_t num_args, T... n )
{
    if constexpr ( N != 0 )
    {
        if ( num_args )
            return call_any<N - 1>( fn, args + 1, num_args - 1, n..., *args );
        else
            return ( ( uint64_t( __stdcall* )( T... ) ) fn )( n... );
    }
    __debugbreak();
    return 0;
}

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
    if ( n >= 16 )
        luaL_error( L, "Too many arguments provided %d vs maximum of 16\n", n );

    // Create the call frame.
    //
    uint64_t* frame = ( uint64_t* ) ( n ? alloca( n * sizeof( void* ) ) : nullptr );
    for ( int i = 0; i < n; i++ )
        frame[ i ] = lua_asintrinsic( L, i + 2 );

    // Invoke the call helper.
    //
    uint64_t result = call_any<>( fn->address, frame, n );

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