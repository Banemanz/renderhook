//
// Created by peter on 19.01.2021.
//
#pragma once
#include "common_headers.h"

#include <cstdint>

namespace rh::rw::engine
{
class SharedMemoryTaskQueue;
class StartSystemCmdImpl
{
  public:
    StartSystemCmdImpl( SharedMemoryTaskQueue &task_queue );
    bool        Invoke( HWND window );
    static void RegisterCallHandler( SharedMemoryTaskQueue &task_queue );

  private:
    SharedMemoryTaskQueue &TaskQueue;
};
} // namespace rh::rw::engine