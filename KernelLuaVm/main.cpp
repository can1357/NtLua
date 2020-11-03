#include "crt/crt.h"
#include <ntifs.h>
#include "logger.hpp"
#include "lua/state.hpp"
#include "lua/native_function.hpp"
#include "driver_io.hpp"
#include "lua/api.hpp"

// Global Lua context and attaching helpers.
//
lua_State* L = nullptr;

PEPROCESS attached_process = nullptr;
KAPC_STATE apc_state;

namespace lua
{
    static void begin_ctx()
    {
        if ( attached_process )
        {
            if ( PsGetProcessExitStatus( attached_process ) != STATUS_PENDING )
            {
                ObDereferenceObject( attached_process );
                attached_process = nullptr;
            }
            else
            {
                KeStackAttachProcess( attached_process, &apc_state );
            }
        }
    }
    static void end_ctx()
    {
        if ( attached_process )
            KeUnstackDetachProcess( &apc_state );
    }

    bool detach()
    {
        if ( !attached_process )
            return false;
        KeUnstackDetachProcess( &apc_state );
        ObDereferenceObject( attached_process );
        attached_process = nullptr;
        return true;
    }
    bool attach_process( PEPROCESS process )
    {
        if ( ObReferenceObjectSafe( process ) )
        {
            detach();
            attached_process = process;
            KeStackAttachProcess( process, &apc_state );
            return true;
        }
        return false;
    }
    bool attach_pid( uint64_t pid )
    {
        PEPROCESS process = nullptr;
        PsLookupProcessByProcessId( ( HANDLE ) pid, &process );
        if ( !process ) 
            return false;

        detach();
        attached_process = process;
        KeStackAttachProcess( process, &apc_state );
        return true;
    }
};



// Device control handler.
//
NTSTATUS device_control( PDEVICE_OBJECT device_object, PIRP irp )
{
    // If current control code is NTLUA_RUN:
    //
    PIO_STACK_LOCATION irp_sp = IoGetCurrentIrpStackLocation( irp );
    if ( irp_sp->Parameters.DeviceIoControl.IoControlCode == NTLUA_RUN )
    {
        const char* input = ( const char* ) irp->AssociatedIrp.SystemBuffer;
        ntlua_result* result = ( ntlua_result* ) irp->AssociatedIrp.SystemBuffer;

        size_t input_length = irp_sp->Parameters.DeviceIoControl.InputBufferLength;
        size_t output_length = irp_sp->Parameters.DeviceIoControl.OutputBufferLength;

        // Begin output size at 0.
        //
        irp->IoStatus.Information = 0;

        // If there is a valid, null-terminated buffer:
        //
        if ( input && input_length && input[ input_length - 1 ] == 0x0 )
        {
            // Reset logger buffers.
            //
            logger::errors.reset();
            logger::logs.reset();

            // Execute the code in the buffer.
            //
            lua::begin_ctx();
            lua::execute( L, input, true );
            lua::end_ctx();

            // Zero out the result.
            //
            result->errors = nullptr;
            result->outputs = nullptr;

            // Declare a helper exporting the buffer from KM memory to UM memory.
            //
            const auto export_buffer = [ ] ( logger::string_buffer& buf )
            {
                // Allocate user-mode memory to hold this buffer.
                //
                void* region = nullptr;
                size_t size = buf.iterator + 1;
                ZwAllocateVirtualMemory( NtCurrentProcess(), ( void** ) &region, 0, &size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
                
                // Copy the buffer if allocation was succesful.
                //
                if ( region )
                {
                    __try
                    {
                        memcpy( region, buf.raw, buf.iterator );
                    }
                    __except ( 1 )
                    {

                    }
                }

                // Reset the buffer and return the newly allocated region.
                //
                buf.reset();
                return ( char* ) region;
            };

            // If we have a valid output buffer:
            //
            if ( output_length >= sizeof( ntlua_result ) )
            {
                if ( logger::errors.iterator )
                    result->errors = export_buffer( logger::errors );
                if ( logger::logs.iterator )
                    result->outputs = export_buffer( logger::logs );
                irp->IoStatus.Information = sizeof( ntlua_result );
            }
        }

        // Declare success and return.
        //
        irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest( irp, IO_NO_INCREMENT );
        return STATUS_SUCCESS;
    }
    else
    {
        // Report failure.
        //
        irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest( irp, IO_NO_INCREMENT );
        return STATUS_UNSUCCESSFUL;
    }
}

// Unloads the driver.
//
void unload_driver( PDRIVER_OBJECT driver )
{
    // Destroy the Lua context.
    //
    lua::destroy( L );

    // Delete the symbolic link.
    //
    UNICODE_STRING sym_link;
    RtlInitUnicodeString( &sym_link, L"\\DosDevices\\NtLua" );
    IoDeleteSymbolicLink( &sym_link );

    // Delete the device object.
    //
    if ( PDEVICE_OBJECT device_object = driver->DeviceObject )
        IoDeleteDevice( device_object );
}

// Execute corporate-level security check.
//
NTSTATUS security_check( PDEVICE_OBJECT device_object, PIRP irp )
{
    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = 0;
    IoCompleteRequest( irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}

// Entry-point.
//
extern "C" NTSTATUS DriverEntry( DRIVER_OBJECT* DriverObject, UNICODE_STRING* RegistryPath )
{
    // Run static initializers.
    //
    crt::initialize();

    // Create a device object.
    //
    UNICODE_STRING device_name;
    RtlInitUnicodeString( &device_name, L"\\Device\\NtLua" );

    PDEVICE_OBJECT device_object;
    NTSTATUS nt_status = IoCreateDevice
    (
        DriverObject,
        0,
        &device_name,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &device_object 
    );
    if ( !NT_SUCCESS( nt_status ) )
        return nt_status;

    // Set callbacks.
    //
    DriverObject->DriverUnload = &unload_driver;
    DriverObject->MajorFunction[ IRP_MJ_CREATE ] = &security_check;
    DriverObject->MajorFunction[ IRP_MJ_CLOSE ] = &security_check;
    DriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] = &device_control;
    
    // Create a symbolic link.
    //
    UNICODE_STRING dos_device;
    RtlInitUnicodeString( &dos_device, L"\\DosDevices\\NtLua" );
    nt_status = IoCreateSymbolicLink( &dos_device, &device_name );
    if ( !NT_SUCCESS( nt_status ) )
    {
        IoDeleteDevice( device_object );
        return nt_status;
    }

    // Initialize Lua.
    //
    L = lua::init();
    lua::expose_api( L );
    return STATUS_SUCCESS;
}