//
// Created by peter on 28.10.2020.
//
#pragma once

#include <cstdint>
class PointLights
{
  public:
    static int32_t AddLight( char a1, float x, float y, float z, float dx,
                             float dy, float dz, float rad, float r, float g,
                             float b, char fogtype, char extrashadows );

    static void Patch();
};