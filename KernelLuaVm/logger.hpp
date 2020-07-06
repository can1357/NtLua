#pragma once
#include <string.h>

// Basic logger implementation.
//
namespace logger
{
    struct string_buffer
    {
        static constexpr size_t buffer_length = 1024 * 1024 * 16;

        char raw[ buffer_length ];
        size_t iterator = 0;

        void append( const void* ptr, size_t n )
        {
            if ( ( iterator + n ) < buffer_length )
            {
                memcpy( raw + iterator, ptr, n );
                iterator += n;
            }
        }

        void reset()
        {
            iterator = 0;
        }
    };

    inline string_buffer logs = {};
    inline string_buffer errors = {};

    template<typename... T>
    static auto error( const char* format, T... args )
    {
        char buffer[ 512 ];
        size_t n = sprintf_s( buffer, 512, format, args... );
        errors.append( buffer, n );
        return n;
    }
    template<typename... T>
    static auto log( const char* format, T... args )
    {
        char buffer[ 512 ];
        size_t n = sprintf_s( buffer, 512, format, args... );
        logs.append( buffer, n );
        return n;
    }
};