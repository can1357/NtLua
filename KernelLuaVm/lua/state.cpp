#include "state.hpp"

namespace lua
{
    struct allocation_header
    {
        uint64_t size;

        static allocation_header* of( void* p ) { return ( ( allocation_header* ) p ) - 1; }

        void* data() { return this + 1; }
        const void* data() const { return this + 1; }
    };

    static void* allocator( void*, void* odata, size_t osize, size_t nsize )
    {
        // If new size is zero:
        //
        if ( !nsize )
        {
            // Deallocate previous if relevant, return null.
            //
            if ( osize ) free( allocation_header::of( odata ) );
            return nullptr;
        }

        // If no old data:
        //
        if ( !odata )
        {
            // Simply allocate as requested and return.
            //
            return ( ( allocation_header* ) malloc( nsize + sizeof( allocation_header ) ) )->data();
        }

        // Resolve allocation header of the old data.
        //
        allocation_header* hdr = allocation_header::of( odata );

        // If it can accomadate curent data and is not "substantially" different in size, return as is.
        //
        if ( hdr->size >= nsize && nsize >= ( hdr->size / 2 ) )
            return odata;

        // Calculate the new "ideal" allocation size.
        //
        nsize = max( min( pow( nsize, 1.2 ), PAGE_SIZE ), nsize );

        // Allocate from non-paged pool and write the size.
        //
        allocation_header* nhdr = ( allocation_header* ) malloc( nsize + sizeof( allocation_header ) );
        nhdr->size = nsize;

        // Relocate the old data and free.
        //
        memcpy( nhdr->data(), odata, min( osize, nsize ) );
        free( hdr );

        // Return pointer to the new data.
        //
        return nhdr->data();
    }

    static int panic( lua_State* L )
    {
        const char* message = lua_tostring( L, -1 );
        logger::error( "Runtime error: %s\n", message );
        longjmp( get_context( L )->panic_jump, 1 );
        return 0;
    }

    // Initializes a Lua state.
    //
    lua_State* init()
    {
        lua_State* L = lua_newstate( &allocator, new lua_context );
        if ( !L ) return nullptr;
        lua_atpanic( L, &panic );
        luaL_openlibs( L );
        return L;
    }

    // Destroys a Lua state.
    //
    void destroy( lua_State* L )
    {
        delete get_context( L );
        lua_close( L );
    }

    // Gets current context from a Lua state.
    //
    lua_context* get_context( lua_State* L )
    {
        void* ctx;
        lua_getallocf( L, &ctx );
        return ( lua_context* ) ctx;
    }

    // Executes code in given Lua state.
    //
    void execute( lua_State* L, const char* code, bool user_input )
    {
        size_t len = strlen( code );
        if ( !len ) return;

        // Reset the Lua stack.
        //
        if ( user_input )
            lua_settop( L, 0 );

        // Guard against Lua panic.
        //
        if ( setjmp( lua::get_context( L )->panic_jump ) == 0 )
        {
            // Try to load the buffer.
            //
            if ( luaL_loadbuffer( L, code, len, "line" ) )
            {
                logger::error( "Lua parser error: %s\n", lua_tostring( L, -1 ) );
                return;
            }

            // Guard against any exceptions.
            //
            __try
            {
                // Guard against any virtual exceptions.
                //
                if ( lua_pcall( L, 0, user_input ? LUA_MULTRET : 0, 0 ) )
                {
                    logger::error( "Lua runtime error: %s\n", lua_tostring( L, -1 ) );
                }
                // If not internal and we have something left on stack:
                //
                else if ( user_input && lua_gettop( L ) > 0 )
                {
                    // Redirect to print.
                    //
                    lua_getglobal( L, "print" );
                    lua_insert( L, 1 );
                    lua_pcall( L, lua_gettop( L ) - 1, 0, 0 );
                }
            }
            __except ( 1 )
            {
                logger::error( "Lua SEH error: %x\n", GetExceptionCode() );
            }
        }
        else
        {
            logger::error( "Lua Panic!" );
        }
    }
};

// Some helpers we need in Lua style.
//
uint64_t lua_asintrinsic( lua_State* L, int i )
{
    switch ( lua_type( L, i ) )
    {
        case LUA_TSTRING:
            return ( uint64_t ) lua_tostring( L, i );
        case LUA_TLIGHTUSERDATA:
        case LUA_TTABLE:
        case LUA_TFUNCTION:
        case LUA_TUSERDATA:
        case LUA_TTHREAD:
            return ( uint64_t ) lua_topointer( L, i );
        case LUA_TNIL:
        case LUA_TBOOLEAN:
        case LUA_TNUMBER:
        default:
            return lua_tounsigned( L, i );
    }
}

void* lua_adressof( lua_State* L, int i )
{
    switch ( lua_type( L, i ) )
    {
        case LUA_TSTRING:
            return ( void* ) lua_tostring( L, i );
        case LUA_TLIGHTUSERDATA:
        case LUA_TTABLE:
        case LUA_TFUNCTION:
        case LUA_TUSERDATA:
        case LUA_TTHREAD:
            return ( void* ) lua_topointer( L, i );
        case LUA_TNIL:
        case LUA_TBOOLEAN:
        case LUA_TNUMBER:
        default:
            return 0;
    }
}
