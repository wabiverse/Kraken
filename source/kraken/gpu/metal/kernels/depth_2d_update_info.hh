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
 * @ingroup GPU.
 * Pixel Magic.
 *
 * Apple Metal | Kernels.
 */

#include "gpu_shader_create_info.hh"

GPU_SHADER_INTERFACE_INFO(depth_2d_update_iface, "").smooth(Type::VEC2, "texCoord_interp");

GPU_SHADER_CREATE_INFO(depth_2d_update_info_base)
  .vertex_in(0, Type::VEC2, "pos")
  .vertex_out(depth_2d_update_iface)
  .fragment_out(0, Type::VEC4, "fragColor")
  .push_constant(Type::VEC2, "extent")
  .push_constant(Type::VEC2, "offset")
  .push_constant(Type::VEC2, "size")
  .push_constant(Type::INT, "mip")
  .sampler(0, ImageType::FLOAT_2D, "source_data", Frequency::PASS)
  .vertex_source("depth_2d_update_vert.glsl");

GPU_SHADER_CREATE_INFO(depth_2d_update_float)
  .fragment_source("depth_2d_update_float_frag.glsl")
  .additional_info("depth_2d_update_info_base")
  .do_static_compilation(true);

GPU_SHADER_CREATE_INFO(depth_2d_update_int24)
  .fragment_source("depth_2d_update_int24_frag.glsl")
  .additional_info("depth_2d_update_info_base")
  .do_static_compilation(true);

GPU_SHADER_CREATE_INFO(depth_2d_update_int32)
  .fragment_source("depth_2d_update_int32_frag.glsl")
  .additional_info("depth_2d_update_info_base")
  .do_static_compilation(true);
