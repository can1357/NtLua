#include "api.hpp"
#include <ntimage.h>

__declspec( dllimport ) extern "C" void RtlPcToFileHeader( void* a1, IMAGE_DOS_HEADER * *a2 );

static void export_func( lua_State* L, const char* name, void* ptr )
{
    native_function* fn = native_function::push( L );
    fn->address = ptr;
    lua_setglobal( L, name );
}

// Read / Write primitives.
//
template<size_t N>
static uint64_t read_n( void* p ) { uint64_t v = 0; memcpy( &v, p, N ); return v; }
template<size_t N>
static void write_n( void* p, uint64_t val ) { memcpy( p, &val, N ); }

// CPU primitives.
//
static uint64_t readmsr( uint32_t msr ) { return __readmsr( msr ); }
static void writemsr( uint32_t msr, uint64_t value ) { return __writemsr( msr, value ); }
static uint64_t readcr0() { return __readcr0(); }
static uint64_t readcr2() { return __readcr2(); }
static uint64_t readcr3() { return __readcr3(); }
static uint64_t readcr4() { return __readcr4(); }
static uint64_t readcr8() { return __readcr8(); }
static void writecr0( uint64_t value ) { return __writecr0( value ); }
static void writecr3( uint64_t value ) { return __writecr3( value ); }
static void writecr4( uint64_t value ) { return __writecr4( value ); }
static void writecr8( uint64_t value ) { return __writecr8( value ); }

// Creates a table that containes every exported address in the given image.
//
static void export_all( lua_State* L, uint64_t image_address )
{
    // Resolve DOS and NT headers.
    //
    PIMAGE_DOS_HEADER dos_header = ( PIMAGE_DOS_HEADER ) image_address;
    PIMAGE_NT_HEADERS nt_headers = ( PIMAGE_NT_HEADERS ) ( image_address + dos_header->e_lfanew );

    // Resole EAT.
    //
    auto& export_dir = nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ];
    uint64_t export_begin = ( uint64_t ) image_address + export_dir.VirtualAddress;
    uint64_t export_end = ( uint64_t ) export_begin + export_dir.Size;
    PIMAGE_EXPORT_DIRECTORY eat = ( PIMAGE_EXPORT_DIRECTORY ) export_begin;

    // Resolve details.
    //
    ULONG* adresses = ( ULONG* ) ( image_address + eat->AddressOfFunctions );
    ULONG* names = ( ULONG* ) ( image_address + eat->AddressOfNames );
    USHORT* ordinals = ( USHORT* ) ( image_address + eat->AddressOfNameOrdinals );

    // Create a table and later on push it to the global named [image_name].
    //
    lua_createtable( L, 0, eat->NumberOfFunctions );
    if ( names )
    {
        // For each named export:
        //
        for ( int i = 0; i < eat->NumberOfNames; i++ )
        {
            // Get the name and the address.
            //
            const char* name = ( const char* ) ( image_address + names[ i ] );
            uint64_t address = image_address + adresses[ ordinals[ i ] ];

            // Skip if redirect.
            //
            if ( address >= ( export_dir.VirtualAddress ) &&
                 address < ( export_dir.VirtualAddress + export_dir.Size ) )
                continue;

            // Insert the table.
            //
            lua_pushstring( L, name );
            native_function* fn = native_function::push( L );
            fn->address = ( void* ) address;
            lua_settable( L, -3 );
        }
    }
}
static int fexport_all( lua_State* L )
{
    export_all( L, lua_asintrinsic( L, 1 ) );
    return 1;
}

// Returns the address of the object on the top of the stack, effectively leaking 
// memory information back to the virtual machine.
//
static int addressof( lua_State* L )
{
    lua_pushinteger( L, ( uint64_t ) lua_adressof( L, 1 ) );
    return 1;
}

// Exposes the NtLua API to the Lua state.
//
void lua::expose_api( lua_State* L )
{
    // Expose native function interface.
    //
    native_function::declare( L );

    // Export memory primitives.
    //
    export_func( L, "memcpy", &memcpy );
    export_func( L, "memset", &memset );
    export_func( L, "memcmp", &memcmp );
    export_func( L, "read1", &read_n<1> );
    export_func( L, "read2", &read_n<2> );
    export_func( L, "read4", &read_n<4> );
    export_func( L, "read8", &read_n<8> );
    export_func( L, "write1", &write_n<1> );
    export_func( L, "write2", &write_n<2> );
    export_func( L, "write4", &write_n<4> );
    export_func( L, "write8", &write_n<8> );

    // Export CPU primitives.
    //
    export_func( L, "readmsr", &readmsr );
    export_func( L, "writemsr", &writemsr );
    export_func( L, "readcr0", &readcr0 );
    export_func( L, "readcr2", &readcr2 );
    export_func( L, "readcr3", &readcr3 );
    export_func( L, "readcr4", &readcr4 );
    export_func( L, "readcr8", &readcr8 );
    export_func( L, "writecr0", &writecr0 );
    export_func( L, "writecr3", &writecr3 );
    export_func( L, "writecr4", &writecr4 );
    export_func( L, "writecr8", &writecr8 );

    // Export misc. functions.
    //
    lua_pushcfunction( L, &addressof );
    lua_setglobal( L, "addressof" );
    lua_pushcfunction( L, &fexport_all );
    lua_setglobal( L, "import" );

    // Export Ntoskrnl.
    //
    UNICODE_STRING unistr = RTL_CONSTANT_STRING( L"MmGetSystemRoutineAddress" );
    void* nt_page = MmGetSystemRoutineAddress( &unistr );

    IMAGE_DOS_HEADER* nt_image_base = nullptr;
    RtlPcToFileHeader( nt_page, &nt_image_base );

    export_all( L, ( uint64_t ) nt_image_base );
    lua_setglobal( L, "nt" );

    // Run common runtime.
    //
    lua::execute
    (
        L,
        #include "../runtime.lua"
    );
}
