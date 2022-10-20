/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Derived from original work by Copyright 2022, Blender Foundation.
 * From the Blender GPU library. (source/blender/gpu).
 *
 * With any additions or modifications specific to Kraken.
 *
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#pragma once

/**
 * @file
 * GPU.
 * Pixel Magic.
 */

#include "KLI_assert.h"

enum eMTLDataType {
  MTL_DATATYPE_CHAR,
  MTL_DATATYPE_CHAR2,
  MTL_DATATYPE_CHAR3,
  MTL_DATATYPE_CHAR4,

  MTL_DATATYPE_UCHAR,
  MTL_DATATYPE_UCHAR2,
  MTL_DATATYPE_UCHAR3,
  MTL_DATATYPE_UCHAR4,

  MTL_DATATYPE_BOOL,
  MTL_DATATYPE_BOOL2,
  MTL_DATATYPE_BOOL3,
  MTL_DATATYPE_BOOL4,

  MTL_DATATYPE_SHORT,
  MTL_DATATYPE_SHORT2,
  MTL_DATATYPE_SHORT3,
  MTL_DATATYPE_SHORT4,

  MTL_DATATYPE_USHORT,
  MTL_DATATYPE_USHORT2,
  MTL_DATATYPE_USHORT3,
  MTL_DATATYPE_USHORT4,

  MTL_DATATYPE_INT,
  MTL_DATATYPE_INT2,
  MTL_DATATYPE_INT3,
  MTL_DATATYPE_INT4,

  MTL_DATATYPE_UINT,
  MTL_DATATYPE_UINT2,
  MTL_DATATYPE_UINT3,
  MTL_DATATYPE_UINT4,

  MTL_DATATYPE_FLOAT,
  MTL_DATATYPE_FLOAT2,
  MTL_DATATYPE_FLOAT3,
  MTL_DATATYPE_FLOAT4,

  MTL_DATATYPE_LONG,
  MTL_DATATYPE_LONG2,
  MTL_DATATYPE_LONG3,
  MTL_DATATYPE_LONG4,

  MTL_DATATYPE_ULONG,
  MTL_DATATYPE_ULONG2,
  MTL_DATATYPE_ULONG3,
  MTL_DATATYPE_ULONG4,

  MTL_DATATYPE_HALF2x2,
  MTL_DATATYPE_HALF2x3,
  MTL_DATATYPE_HALF2x4,
  MTL_DATATYPE_HALF3x2,
  MTL_DATATYPE_HALF3x3,
  MTL_DATATYPE_HALF3x4,
  MTL_DATATYPE_HALF4x2,
  MTL_DATATYPE_HALF4x3,
  MTL_DATATYPE_HALF4x4,

  MTL_DATATYPE_FLOAT2x2,
  MTL_DATATYPE_FLOAT2x3,
  MTL_DATATYPE_FLOAT2x4,
  MTL_DATATYPE_FLOAT3x2,
  MTL_DATATYPE_FLOAT3x3,
  MTL_DATATYPE_FLOAT3x4,
  MTL_DATATYPE_FLOAT4x2,
  MTL_DATATYPE_FLOAT4x3,
  MTL_DATATYPE_FLOAT4x4,

  MTL_DATATYPE_UINT1010102_NORM,
  MTL_DATATYPE_INT1010102_NORM
};

inline uint mtl_get_data_type_size(eMTLDataType type)
{
  switch (type) {
    case MTL_DATATYPE_CHAR:
    case MTL_DATATYPE_UCHAR:
    case MTL_DATATYPE_BOOL:
      return 1;
    case MTL_DATATYPE_CHAR2:
    case MTL_DATATYPE_UCHAR2:
    case MTL_DATATYPE_BOOL2:
    case MTL_DATATYPE_SHORT:
    case MTL_DATATYPE_USHORT:
      return 2;

    case MTL_DATATYPE_CHAR3:
    case MTL_DATATYPE_UCHAR3:
    case MTL_DATATYPE_BOOL3:
      return 3;
    case MTL_DATATYPE_CHAR4:
    case MTL_DATATYPE_UCHAR4:
    case MTL_DATATYPE_INT:
    case MTL_DATATYPE_UINT:
    case MTL_DATATYPE_BOOL4:
    case MTL_DATATYPE_SHORT2:
    case MTL_DATATYPE_USHORT2:
    case MTL_DATATYPE_FLOAT:
    case MTL_DATATYPE_UINT1010102_NORM:
    case MTL_DATATYPE_INT1010102_NORM:
      return 4;

    case MTL_DATATYPE_SHORT3:
    case MTL_DATATYPE_USHORT3:
    case MTL_DATATYPE_SHORT4:
    case MTL_DATATYPE_USHORT4:
    case MTL_DATATYPE_INT2:
    case MTL_DATATYPE_UINT2:
    case MTL_DATATYPE_FLOAT2:
    case MTL_DATATYPE_LONG:
    case MTL_DATATYPE_ULONG:
    case MTL_DATATYPE_HALF2x2:
      return 8;

    case MTL_DATATYPE_HALF3x2:
      return 12;

    case MTL_DATATYPE_INT3:
    case MTL_DATATYPE_INT4:
    case MTL_DATATYPE_UINT3:
    case MTL_DATATYPE_UINT4:
    case MTL_DATATYPE_FLOAT3:
    case MTL_DATATYPE_FLOAT4:
    case MTL_DATATYPE_LONG2:
    case MTL_DATATYPE_ULONG2:
    case MTL_DATATYPE_HALF2x3:
    case MTL_DATATYPE_HALF2x4:
    case MTL_DATATYPE_HALF4x2:
      return 16;

    case MTL_DATATYPE_HALF3x3:
    case MTL_DATATYPE_HALF3x4:
    case MTL_DATATYPE_FLOAT3x2:
      return 24;

    case MTL_DATATYPE_LONG3:
    case MTL_DATATYPE_LONG4:
    case MTL_DATATYPE_ULONG3:
    case MTL_DATATYPE_ULONG4:
    case MTL_DATATYPE_HALF4x3:
    case MTL_DATATYPE_HALF4x4:
    case MTL_DATATYPE_FLOAT2x3:
    case MTL_DATATYPE_FLOAT2x4:
    case MTL_DATATYPE_FLOAT4x2:
      return 32;

    case MTL_DATATYPE_FLOAT3x3:
    case MTL_DATATYPE_FLOAT3x4:
      return 48;

    case MTL_DATATYPE_FLOAT4x3:
    case MTL_DATATYPE_FLOAT4x4:
      return 64;
    default:
      KLI_assert(false);
      return 0;
  };
}

inline uint mtl_get_data_type_alignment(eMTLDataType type)
{
  switch (type) {
    case MTL_DATATYPE_CHAR:
    case MTL_DATATYPE_UCHAR:
    case MTL_DATATYPE_BOOL:
      return 1;
    case MTL_DATATYPE_CHAR2:
    case MTL_DATATYPE_UCHAR2:
    case MTL_DATATYPE_BOOL2:
    case MTL_DATATYPE_SHORT:
    case MTL_DATATYPE_USHORT:
      return 2;

    case MTL_DATATYPE_CHAR3:
    case MTL_DATATYPE_UCHAR3:
    case MTL_DATATYPE_BOOL3:
      return 3;
    case MTL_DATATYPE_CHAR4:
    case MTL_DATATYPE_UCHAR4:
    case MTL_DATATYPE_INT:
    case MTL_DATATYPE_UINT:
    case MTL_DATATYPE_BOOL4:
    case MTL_DATATYPE_SHORT2:
    case MTL_DATATYPE_USHORT2:
    case MTL_DATATYPE_FLOAT:
    case MTL_DATATYPE_HALF2x2:
    case MTL_DATATYPE_HALF3x2:
    case MTL_DATATYPE_HALF4x2:
    case MTL_DATATYPE_UINT1010102_NORM:
    case MTL_DATATYPE_INT1010102_NORM:
      return 4;

    case MTL_DATATYPE_SHORT3:
    case MTL_DATATYPE_USHORT3:
    case MTL_DATATYPE_SHORT4:
    case MTL_DATATYPE_USHORT4:
    case MTL_DATATYPE_INT2:
    case MTL_DATATYPE_UINT2:
    case MTL_DATATYPE_FLOAT2:
    case MTL_DATATYPE_LONG:
    case MTL_DATATYPE_ULONG:
    case MTL_DATATYPE_HALF2x3:
    case MTL_DATATYPE_HALF2x4:
    case MTL_DATATYPE_HALF3x3:
    case MTL_DATATYPE_HALF3x4:
    case MTL_DATATYPE_HALF4x3:
    case MTL_DATATYPE_HALF4x4:
    case MTL_DATATYPE_FLOAT2x2:
    case MTL_DATATYPE_FLOAT3x2:
    case MTL_DATATYPE_FLOAT4x2:
      return 8;

    case MTL_DATATYPE_INT3:
    case MTL_DATATYPE_INT4:
    case MTL_DATATYPE_UINT3:
    case MTL_DATATYPE_UINT4:
    case MTL_DATATYPE_FLOAT3:
    case MTL_DATATYPE_FLOAT4:
    case MTL_DATATYPE_LONG2:
    case MTL_DATATYPE_ULONG2:
    case MTL_DATATYPE_FLOAT2x3:
    case MTL_DATATYPE_FLOAT2x4:
    case MTL_DATATYPE_FLOAT3x3:
    case MTL_DATATYPE_FLOAT3x4:
    case MTL_DATATYPE_FLOAT4x3:
    case MTL_DATATYPE_FLOAT4x4:
      return 16;

    case MTL_DATATYPE_LONG3:
    case MTL_DATATYPE_LONG4:
    case MTL_DATATYPE_ULONG3:
    case MTL_DATATYPE_ULONG4:
      return 32;

    default:
      KLI_assert_msg(false, "Unrecognized MTL datatype.");
      return 0;
  };
}
