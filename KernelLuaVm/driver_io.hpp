#pragma once

// Assuming the platform specific header is included already.
#define NTLUA_RUN   CTL_CODE( 0x13, 0x37, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define NTLUA_RESET CTL_CODE( 0x13, 0x38, METHOD_BUFFERED, FILE_ANY_ACCESS )

// Shared structures.
//
struct ntlua_result
{
    char* errors;
    char* outputs;
};