#pragma once
#include "../crt/crt.h"
#include <ntifs.h>
#include "../logger.hpp"

// Build lua as cpp 
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

namespace lua
{
    struct lua_context
    {
        // Jump buffer taken on Lua panic.
        //
        jmp_buf panic_jump = {};
    };

    // Initializes a Lua state.
    //
    lua_State* init();

    // Destroys a Lua state.
    //
    void destroy( lua_State* L );

    // Gets current context from a Lua state.
    //
    lua_context* get_context( lua_State* L );

    // Executes code in given Lua state.
    //
    void execute( lua_State* L, const char* code, bool user_input = false );
};

// Some helpers we need in Lua style.
//
uint64_t lua_asintrinsic( lua_State* L, int i );
void* lua_adressof( lua_State* L, int i );
