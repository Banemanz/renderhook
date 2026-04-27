#pragma once
#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <system_error>
#include <type_traits>

static void Patch( INT_PTR address, void *data, SIZE_T size )
{
    auto adr_ptr = reinterpret_cast<void *>( address );

    unsigned long protect[2];
    VirtualProtect( adr_ptr, size, PAGE_EXECUTE_READWRITE, &protect[0] );
    memcpy( adr_ptr, data, size );
    VirtualProtect( adr_ptr, size, protect[0], &protect[1] );
}

/**
 * \brief
 * \param address
 * \param value
 */
static void SetPointer( INT_PTR address, void *value )
{
    Patch( address, &value, sizeof( void * ) );
}

inline static void SetInt( INT_PTR address, int value )
{
    Patch( address, &value, sizeof( int ) );
}

inline static void RedirectCall( INT_PTR address, void *func )
{
    auto func_iptr = reinterpret_cast<INT_PTR>( func );

    unsigned char op = 0xE8;
    int32_t       rel = static_cast<int32_t>( func_iptr - ( address + 5 ) );

    Patch( address, &op, sizeof( op ) );
    Patch( ( address + 1 ), &rel, sizeof( rel ) );
}

inline static void RedirectJump( INT_PTR address, void *func )
{
    auto func_iptr = reinterpret_cast<INT_PTR>( func );

    unsigned char op = 0xE9;
    int32_t       rel = static_cast<int32_t>( func_iptr - ( address + 5 ) );

    Patch( address, &op, sizeof( op ) );
    Patch( ( address + 1 ), &rel, sizeof( rel ) );
}

inline static void Nop( INT_PTR address, SIZE_T size )
{
    auto adr_ptr = reinterpret_cast<void *>( address );

    unsigned long protect[2];
    VirtualProtect( adr_ptr, size, PAGE_EXECUTE_READWRITE, &protect[0] );
    memset( adr_ptr, 0x90, size );
    VirtualProtect( adr_ptr, size, protect[0], &protect[1] );
}

template <typename RetType, uint32_t vtable_id, typename C, typename... Args>
auto InMemoryVirtualFunc( C _this, Args... args )
{
    using return_type = RetType( __thiscall * )( C, Args... );
    auto address      = ( *reinterpret_cast<void ***>( _this ) )[vtable_id];
    if constexpr ( std::is_void_v<RetType> )
        reinterpret_cast<return_type>( address )( _this, args... );
    else
        return reinterpret_cast<return_type>( address )( _this, args... );
}

template <typename RetType, typename... Args>
auto InMemoryFuncCall( uint32_t address, Args... args )
{
    using return_type = RetType( __cdecl * )( Args... );
    if constexpr ( std::is_void_v<RetType> )
        reinterpret_cast<return_type>( address )( args... );
    else
        return reinterpret_cast<return_type>( address )( args... );
}
