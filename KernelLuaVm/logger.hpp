#pragma once
#include <string.h>

extern "C" {
    __declspec( dllimport ) int sprintf_s( char* buffer, size_t sizeOfBuffer, const char* format, ... );
};

// Basic logger implementation.
//
namespace logger
{
    struct string_buffer
    {
        static constexpr size_t buffer_length = 1024 * 1024 * 16;

        char raw[ buffer_length ];
        size_t iterator = 0;

        template<typename... T>
        int append( const char* format, T... args )
        {
            int result = sprintf_s( &raw[ iterator ], buffer_length - iterator, format, args... );
            if ( result > 0 )
                iterator += result;
            return result;
        }

        int append_raw( const char* data, size_t len )
        {
            if ( len > ( buffer_length - iterator ) )
                len = buffer_length - iterator;
            memcpy( &raw[ iterator ], data, len );
            iterator += len;
            return len;
        }

        void reset()
        {
            iterator = 0;
        }
    };

    inline string_buffer logs = {};
    inline string_buffer errors = {};

    template<typename... T> inline int error( const char* format, T... args ) { return errors.append( format, args... ); }
    template<typename... T> inline int log( const char* format, T... args ) { return logs.append( format, args... ); }
};