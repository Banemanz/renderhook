//
// Created by peter on 24.02.2021.
//

#include "imgui_win32_driver_handler.h"
#include <data_desc/imgui_input_state.h>

#include <imgui.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#if !defined( ImGuiKey_KeypadEnter ) && defined( ImGuiKey_KeyPadEnter )
#define ImGuiKey_KeypadEnter ImGuiKey_KeyPadEnter
#endif

namespace rh::rw::engine
{

HWND             Window{};
LARGE_INTEGER    Time{};
LARGE_INTEGER    TicksPerSecond{};
ImGuiMouseCursor LastMouseCursor = ImGuiMouseCursor_COUNT;

bool ImGuiWin32DriverHandler::Init( void *hwnd )
{
    if ( !::QueryPerformanceFrequency( &TicksPerSecond ) )
        return false;
    if ( !::QueryPerformanceCounter( &TicksPerSecond ) )
        return false;
    Window = static_cast<HWND>( hwnd );

    ImGuiIO &io = ImGui::GetIO();
    io.BackendFlags |=
        ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor()
    // values (optional)
    io.BackendFlags |=
        ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos
    // requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_win32_rh";

    return true;
}

void ImGuiWin32DriverHandler::Shutdown()
{
    Window                  = nullptr;
    Time.QuadPart           = 0;
    TicksPerSecond.QuadPart = 0;
    LastMouseCursor         = ImGuiMouseCursor_COUNT;
}

void ImGuiWin32DriverHandler::UpdateMousePos()
{
    ImGuiIO &io = ImGui::GetIO();

    // Set OS mouse position if requested (rarely used, only when
    // ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if ( io.WantSetMousePos )
    {
        POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
        if ( ::ClientToScreen( Window, &pos ) )
            ::SetCursorPos( pos.x, pos.y );
    }

    // Set mouse position
    io.MousePos = ImVec2( -FLT_MAX, -FLT_MAX );
    POINT pos;
    if ( HWND active_window = ::GetForegroundWindow() )
        if ( active_window == Window || ::IsChild( active_window, Window ) )
            if ( ::GetCursorPos( &pos ) && ::ScreenToClient( Window, &pos ) )
                io.MousePos = ImVec2( (float)pos.x, (float)pos.y );
}

bool ImGuiWin32DriverHandler::UpdateMouseCursor()
{
    ImGuiIO &io = ImGui::GetIO();
    if ( io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange )
        return false;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if ( imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor )
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no
        // cursor
        ::SetCursor( nullptr );
    }
    else
    {
        // Show OS mouse cursor
        LPTSTR win32_cursor;
        switch ( imgui_cursor )
        {
        default:
        case ImGuiMouseCursor_Arrow: win32_cursor = IDC_ARROW; break;

        case ImGuiMouseCursor_TextInput: win32_cursor = IDC_IBEAM; break;
        case ImGuiMouseCursor_ResizeAll: win32_cursor = IDC_SIZEALL; break;
        case ImGuiMouseCursor_ResizeEW: win32_cursor = IDC_SIZEWE; break;
        case ImGuiMouseCursor_ResizeNS: win32_cursor = IDC_SIZENS; break;
        case ImGuiMouseCursor_ResizeNESW: win32_cursor = IDC_SIZENESW; break;
        case ImGuiMouseCursor_ResizeNWSE: win32_cursor = IDC_SIZENWSE; break;
        case ImGuiMouseCursor_Hand: win32_cursor = IDC_HAND; break;
        case ImGuiMouseCursor_NotAllowed: win32_cursor = IDC_NO; break;
        }
        ::SetCursor( ::LoadCursor( nullptr, win32_cursor ) );
    }
    return true;
}

void ImGuiWin32DriverHandler::NewFrame( const ImGuiInputState &state )
{
    ImGui::GetCurrentContext();
    ImGuiIO &io = ImGui::GetIO();
    /* IM_ASSERT( io.Fonts->IsBuilt() &&
                "Font atlas not built! It is generally built by the renderer "
                "backend. Missing call to renderer NewFrame() function? e.g. "
                "ImGui_ImplVulkan_NewFrame()." );*/
    // Setup display size (every frame to accommodate for window resizing)
    RECT rect = { 0, 0, 0, 0 };
    if ( !::GetClientRect( Window, &rect ) )
        assert( false && "Failed to get client rect for renderer window!" );
    io.DisplaySize = ImVec2( (float)( rect.right - rect.left ),
                             (float)( rect.bottom - rect.top ) );
    // Setup time step
    LARGE_INTEGER current_time = { 0ll };
    ::QueryPerformanceCounter( &current_time );
    io.DeltaTime = (float)( current_time.QuadPart - Time.QuadPart ) /
                   TicksPerSecond.QuadPart;
    Time = current_time;

    // Read keyboard/mouse input
    io.AddKeyEvent( ImGuiMod_Ctrl, state.KeyCtrl );
    io.AddKeyEvent( ImGuiMod_Shift, state.KeyShift );
    io.AddKeyEvent( ImGuiMod_Alt, state.KeyAlt );
    io.AddKeyEvent( ImGuiMod_Super, false );

    io.AddKeyEvent( ImGuiKey_Tab, state.KeysDown[VK_TAB] );
    io.AddKeyEvent( ImGuiKey_LeftArrow, state.KeysDown[VK_LEFT] );
    io.AddKeyEvent( ImGuiKey_RightArrow, state.KeysDown[VK_RIGHT] );
    io.AddKeyEvent( ImGuiKey_UpArrow, state.KeysDown[VK_UP] );
    io.AddKeyEvent( ImGuiKey_DownArrow, state.KeysDown[VK_DOWN] );
    io.AddKeyEvent( ImGuiKey_PageUp, state.KeysDown[VK_PRIOR] );
    io.AddKeyEvent( ImGuiKey_PageDown, state.KeysDown[VK_NEXT] );
    io.AddKeyEvent( ImGuiKey_Home, state.KeysDown[VK_HOME] );
    io.AddKeyEvent( ImGuiKey_End, state.KeysDown[VK_END] );
    io.AddKeyEvent( ImGuiKey_Insert, state.KeysDown[VK_INSERT] );
    io.AddKeyEvent( ImGuiKey_Delete, state.KeysDown[VK_DELETE] );
    io.AddKeyEvent( ImGuiKey_Backspace, state.KeysDown[VK_BACK] );
    io.AddKeyEvent( ImGuiKey_Space, state.KeysDown[VK_SPACE] );
    io.AddKeyEvent( ImGuiKey_Enter, state.KeysDown[VK_RETURN] );
    io.AddKeyEvent( ImGuiKey_Escape, state.KeysDown[VK_ESCAPE] );
    io.AddKeyEvent( ImGuiKey_KeypadEnter, state.KeysDown[VK_RETURN] );
    io.AddKeyEvent( ImGuiKey_A, state.KeysDown['A'] );
    io.AddKeyEvent( ImGuiKey_C, state.KeysDown['C'] );
    io.AddKeyEvent( ImGuiKey_V, state.KeysDown['V'] );
    io.AddKeyEvent( ImGuiKey_X, state.KeysDown['X'] );
    io.AddKeyEvent( ImGuiKey_Y, state.KeysDown['Y'] );
    io.AddKeyEvent( ImGuiKey_Z, state.KeysDown['Z'] );

    for ( auto i = 0; i < 5; i++ )
        io.AddMouseButtonEvent( i, state.MouseDown[i] );
    io.AddMouseWheelEvent( state.MouseWheelH, state.MouseWheel );

    // Update OS mouse position
    UpdateMousePos();

    io.AddMousePosEvent( state.MousePos[0], state.MousePos[1] );

    // Update OS mouse cursor with the cursor requested by imgui
    ImGuiMouseCursor mouse_cursor =
        io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if ( LastMouseCursor != mouse_cursor )
    {
        LastMouseCursor = mouse_cursor;
        UpdateMouseCursor();
    }
}
} // namespace rh::rw::engine
