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
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * @ingroup IMBUF
 * Image Manipulation.
 */

#ifndef GPU_SHADER
#  include "GPU_shader_shared_utils.h"
#endif

struct OCIO_GPUCurveMappingParameters
{
  /* Curve mapping parameters
   *
   * See documentation for OCIO_CurveMappingSettings to get fields descriptions.
   * (this ones pretty much copies stuff from C structure.)
   */
  float4 mintable;
  float4 range;
  float4 ext_in_x;
  float4 ext_in_y;
  float4 ext_out_x;
  float4 ext_out_y;
  float4 first_x;
  float4 first_y;
  float4 last_x;
  float4 last_y;
  float4 black;
  float4 bwmul;
  int lut_size;
  int use_extend_extrapolate;
  int _pad0;
  int _pad1;
};

struct OCIO_GPUParameters
{
  float dither;
  float scale;
  float exponent;
  bool1 use_predivide;
  bool1 use_overlay;
  int _pad0;
  int _pad1;
  int _pad2;
};
