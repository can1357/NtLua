#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>
#include <mutex>
#include "../KernelLuaVm/driver_io.hpp"

HANDLE device = CreateFileA
(
    "\\\\.\\NtLua",
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL
);
bool execute( const char* str, bool silent )
{
    // Issue the IOCTL.
    //
    DWORD discarded = 0;
    ntlua_result result = { nullptr, nullptr };
    DeviceIoControl( device, NTLUA_RUN, ( void* ) str, strlen( str ) + 1, &result, sizeof( result ), &discarded, nullptr );
    bool had_result = result.outputs != nullptr;

    // If silent, free result and return.
    //
    if ( silent )
    {
        if ( result.outputs ) VirtualFree( result.outputs, 0, MEM_RELEASE );
        if ( result.errors ) VirtualFree( result.errors, 0, MEM_RELEASE );
    }
    // Print each buffer to the console.
    //
    else
    {
        for ( auto& [buffer, color] : { std::pair{ result.errors, 12 },
                                        std::pair{ result.outputs, 15 } } )
        {
            if ( !buffer ) continue;
            SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), color );
            puts( buffer );
            VirtualFree( buffer, 0, MEM_RELEASE );
        }
    }
    return had_result;
}

void worker_thread()
{
    bool prev_success = false;
    while ( 1 )
    {
        Sleep( prev_success ? 100 : 5000 );
        static constexpr char worker_script[] = R"(
            if worker then 
                worker() 
                print("-")
            end
        )";
        prev_success = execute( worker_script, true );
    }
}


int main( int argc, const char** argv )
{
    if ( device == INVALID_HANDLE_VALUE ) return 1;

    // If any arguments are given, assume they're lua files and execute them.
    //
    if ( argc >= 2 )
    {
        for ( size_t n = 1; n != argc; n++ )
        {
            printf( "Running '%s'...\n", argv[ n ] );

            std::ifstream fs( argv[ n ] );
            std::string buffer{ std::istreambuf_iterator<char>( fs ), {} };
            execute( buffer.data(), false );
        }
        return 0;
    }

    // Start the worker thread.
    //
    std::thread thr( &worker_thread );

    // Enter REPL:
    //
    while ( 1 )
    {
        // Reset colors and ask user for input.
        //
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 7 );
        std::string buffer;
        std::cout << "=> ";
        std::getline( std::cin, buffer );

        // While shift is being held, allow multiple lines to be inputted.
        //
        while ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
        {
            std::string buffer2;
            std::cout << "   ";
            std::getline( std::cin, buffer2 );
            buffer += "\n" + buffer2;
        }

        // Handle special commands:
        //
        if ( buffer == "clear" )
        {
            system( "cls" );
        }
        else if ( buffer == "cmd" )
            return system( "cmd" );
        else if ( buffer == "exit" )
            return 0;
        else if ( buffer == "reset" )
        {
            DWORD discarded = 0;
            DeviceIoControl(
                device,
                NTLUA_RESET,
                &buffer[ 0 ], buffer.size() + 1,
                &discarded, sizeof( discarded ),
                &discarded, nullptr
            );
        }
        else
        {
            execute( buffer.data(), false );
        }
    }
}