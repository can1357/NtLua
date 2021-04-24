#include <Windows.h>
#include <string>
#include <iostream>
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

void worker_thread()
{
    bool prev_success = false;
    while ( 1 )
    {
        Sleep( prev_success ? 100 : 5000 );

        ntlua_result result = { nullptr, nullptr };
        char worker_script[] = R"(
            if worker then 
                worker() 
                print("-")
            end
        )";

        DWORD discarded = 0;
        DeviceIoControl(
            device,
            NTLUA_RUN,
            worker_script, sizeof( worker_script ),
            &result, sizeof( result ),
            &discarded, nullptr
        );

        if ( result.outputs ) VirtualFree( result.outputs, 0, MEM_RELEASE );
        if ( result.errors ) VirtualFree( result.errors, 0, MEM_RELEASE );
        prev_success = !result.outputs;
    }
}

int main()
{
    if ( device == INVALID_HANDLE_VALUE ) return 1;
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
            continue;
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
            continue;
        }

        // Send IOCTL.
        //
        ntlua_result result = { nullptr, nullptr };
        DWORD discarded = 0;
        DeviceIoControl( 
            device,
            NTLUA_RUN,
            &buffer[0], buffer.size() + 1,
            &result, sizeof( result ),
            &discarded, nullptr
        );

        // Print each buffer if relevant.
        //
        for ( auto& [buffer, color] : { std::pair{ result.errors, 12 }, 
                                        std::pair{ result.outputs, 15 } } )
        {
            if ( !buffer ) continue;
            SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), color );
            puts( buffer );
            VirtualFree( buffer, 0, MEM_RELEASE );
        }
    }
}