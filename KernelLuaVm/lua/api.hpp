#pragma once
#include "../crt/crt.h"
#include "native_function.hpp"

namespace lua
{
	// Exposes the NtLua API to the Lua state.
	//
	void expose_api( lua_State* L );
};
