#include <Windows.h>
#include <string>
#include <iostream>
#include <sstream>
#include <tuple>
#include "../KernelLuaVm/driver_io.hpp"

int main()
{
    // Create a handle to the device.
    //
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
    if ( device == INVALID_HANDLE_VALUE ) return 1;

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

        // Send IOCTL.
        //
        ntlua_result result = { 
            nullptr, 
            nullptr 
        };

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