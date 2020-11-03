#pragma once
#include "../crt/crt.h"
#include "native_function.hpp"

namespace lua
{
	bool attach_process( PEPROCESS process );
	bool attach_pid( uint64_t pid );
	bool detach();

	// Exposes the NtLua API to the Lua state.
	//
	void expose_api( lua_State* L );
};
