//
// Created by peter on 20.01.2021.
//

#include "render_client.h"
#include <Engine/EngineConfigBlock.h>
#include <rw_engine/rh_backend/material_backend.h>
#include <rw_engine/rh_backend/raster_backend.h>

namespace rh::rw::engine
{
std::unique_ptr<RenderClient> gRenderClient = nullptr;

/**
 * RenderClient RenderWare plugins storage
 */
class ClientPlugins
{
  public:
    explicit ClientPlugins( const PluginPtrTable &plugin_cb )
        : Raster( plugin_cb ), Material( plugin_cb )
    {
    }

  private:
    BackendRasterPlugin   Raster;
    BackendMaterialPlugin Material;
};

RenderClient::RenderClient()
{
    /// initialize SM task queue
    TaskQueue =
        std::make_unique<SharedMemoryTaskQueue>( SharedMemoryTaskQueueInfo{
            .mName = "RenderHookTaskQueue",
            .mSize = 1024 * 1024 *
                     rh::engine::EngineConfigBlock::It.SharedMemorySizeMB,
            .mOwner = true } );
}

RenderClient::~RenderClient()
{
    TaskQueue->SendExitEvent();
    TaskQueue.reset();
    if ( RenderDriverStarted && RenderDriverProcess.hProcess )
        TerminateProcess( RenderDriverProcess.hProcess, 0 );
    if ( RenderDriverStarted && RenderDriverProcess.hThread )
        CloseHandle( RenderDriverProcess.hThread );
    if ( RenderDriverStarted && RenderDriverProcess.hProcess )
        CloseHandle( RenderDriverProcess.hProcess );
}

bool RenderClient::RegisterPlugins( const PluginPtrTable &plugin_cb )
{
    Plugins = std::make_unique<ClientPlugins>( plugin_cb );
    return true;
}

bool RenderClient::StartRenderDriverProcess()
{
    if ( RenderDriverStarted )
        return true;

    if ( IPCSettings::mProcessName.empty() )
    {
        debug::DebugLogger::Log(
            "Render driver process name is empty.",
            debug::LogLevel::Error );
        return false;
    }

    STARTUPINFOA start_info{ .cb = sizeof( start_info ) };
    if ( !CreateProcessA( IPCSettings::mProcessName.c_str(), nullptr, nullptr,
                          nullptr, false, 0, nullptr, nullptr, &start_info,
                          &RenderDriverProcess ) )
    {
        debug::DebugLogger::Log(
            "Failed to start render driver process: " +
                IPCSettings::mProcessName,
            debug::LogLevel::Error );
        return false;
    }

    RenderDriverStarted = true;
    return true;
}

void RenderClient::EnsureRenderDriverStarted()
{
    StartRenderDriverProcess();
}

} // namespace rh::rw::engine
