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

static uint64_t readdr0() { return __readdr(0); }
static uint64_t readdr1() { return __readdr(1); }
static uint64_t readdr2() { return __readdr(2); }
static uint64_t readdr3() { return __readdr(3); }
static uint64_t readdr6() { return __readdr(6); }
static uint64_t readdr7() { return __readdr(7); }
static void writedr0( uint64_t value ) { return __writedr( 0, value ); }
static void writedr1( uint64_t value ) { return __writedr( 1, value ); }
static void writedr2( uint64_t value ) { return __writedr( 2, value ); }
static void writedr3( uint64_t value ) { return __writedr( 3, value ); }
static void writedr6( uint64_t value ) { return __writedr( 6, value ); }
static void writedr7( uint64_t value ) { return __writedr( 7, value ); }

static uint8_t inbyte( uint16_t port ) { return __inbyte( port ); }
static uint16_t inword( uint16_t port ) { return __inword( port ); }
static uint32_t indword( uint16_t port ) { return __indword( port ); }
static void outbyte( uint16_t port, uint8_t value ) { return __outbyte( port, value ); }
static void outword( uint16_t port, uint16_t value ) { return __outword( port, value ); }
static void outdword( uint16_t port, uint32_t value ) { return __outdword( port, value ); }

static uint64_t readtsc() { return __rdtsc(); }
static uint64_t readpmc( uint32_t pmc ) { return __readpmc( pmc ); }
static uint64_t readgsbase() { return _readgsbase_u64(); }
static uint64_t readfsbase() { return _readfsbase_u64(); }

static void* read_svirt( const void* src, size_t n )
{
    static auto _MmCopyMemory = [ ] ()
    {
        UNICODE_STRING name = RTL_CONSTANT_STRING( L"MmCopyMemory" );
        return ( decltype( &MmCopyMemory ) ) MmGetSystemRoutineAddress( &name );
    }();
    if ( !_MmCopyMemory ) return nullptr;

    void* buffer = malloc( n );
    if ( !buffer ) return nullptr;

    size_t counter = 0;
    _MmCopyMemory(
        buffer,
        *( MM_COPY_ADDRESS* ) &src,
        n,
        MM_COPY_MEMORY_VIRTUAL,
        &counter
    );
    if ( counter == n )
        return buffer;
    free( buffer );
    return nullptr;
}
static void* read_sphys( uint64_t src, size_t n )
{
    static auto _MmCopyMemory = [ ] ()
    {
        UNICODE_STRING name = RTL_CONSTANT_STRING( L"MmCopyMemory" );
        return ( decltype( &MmCopyMemory ) ) MmGetSystemRoutineAddress( &name );
    }();
    if ( !_MmCopyMemory ) return nullptr;

    void* buffer = malloc( n );
    if ( !buffer ) return nullptr;

    size_t counter = 0;
    _MmCopyMemory(
        buffer,
        *( MM_COPY_ADDRESS* ) &src,
        n,
        MM_COPY_MEMORY_PHYSICAL,
        &counter
    );
    if ( counter == n )
        return buffer;
    free( buffer );
    return nullptr;
}

template<size_t N>
static uint64_t read_pn( uint64_t p ) 
{ 
    static auto _MmCopyMemory = [ ] ()
    {
        UNICODE_STRING name = RTL_CONSTANT_STRING( L"MmCopyMemory" );
        return ( decltype( &MmCopyMemory ) ) MmGetSystemRoutineAddress( &name );
    }();
    if ( !_MmCopyMemory ) return 0xFFFFFFFFFFFFFFFF;

    uint64_t v = 0;
    size_t counter = 0;
    _MmCopyMemory(
        &v,
        *( MM_COPY_ADDRESS* ) &p,
        N,
        MM_COPY_MEMORY_PHYSICAL,
        &counter
    );
    return v; 
}

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

    export_func( L, "readdr0", &readdr0 );
    export_func( L, "readdr1", &readdr1 );
    export_func( L, "readdr2", &readdr2 );
    export_func( L, "readdr3", &readdr3 );
    export_func( L, "readdr6", &readdr6 );
    export_func( L, "readdr7", &readdr7 );
    export_func( L, "writedr0", &writedr0 );
    export_func( L, "writedr1", &writedr1 );
    export_func( L, "writedr2", &writedr2 );
    export_func( L, "writedr3", &writedr3 );
    export_func( L, "writedr6", &writedr6 );
    export_func( L, "writedr7", &writedr7 );

    export_func( L, "inbyte", &inbyte );
    export_func( L, "inword", &inword );
    export_func( L, "indword", &indword );

    export_func( L, "outbyte", &outbyte );
    export_func( L, "outword", &outword );
    export_func( L, "outdword", &outdword );

    export_func( L, "readtsc", &readtsc );
    export_func( L, "readpmc", &readpmc );
    export_func( L, "readgsbase", &readgsbase );
    export_func( L, "readfsbase", &readfsbase );

    // Export simpler helpers.
    //
    export_func( L, "readps", &read_sphys );
    export_func( L, "readvs", &read_svirt );

    export_func( L, "attach_process", &attach_process );
    export_func( L, "attach_pid", &attach_pid );
    export_func( L, "detach", &detach );
    export_func( L, "readp1", &read_pn<1> );
    export_func( L, "readp2", &read_pn<2> );
    export_func( L, "readp4", &read_pn<4> );
    export_func( L, "readp8", &read_pn<8> );

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
