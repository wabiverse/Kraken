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
 * From the Blender Library. (source/blender/blenlib).
 *
 * With any additions or modifications specific to Kraken.
 *
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#pragma once

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#include "KLI_math_inline.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************** Blending Modes **********************
 * - byte function assume straight alpha
 * - float functions assume premultiplied alpha
 */

MINLINE void blend_color_mix_byte(unsigned char dst[4],
                                  const unsigned char src1[4],
                                  const unsigned char src2[4]);
MINLINE void blend_color_add_byte(unsigned char dst[4],
                                  const unsigned char src1[4],
                                  const unsigned char src2[4]);
MINLINE void blend_color_sub_byte(unsigned char dst[4],
                                  const unsigned char src1[4],
                                  const unsigned char src2[4]);
MINLINE void blend_color_mul_byte(unsigned char dst[4],
                                  const unsigned char src1[4],
                                  const unsigned char src2[4]);
MINLINE void blend_color_lighten_byte(unsigned char dst[4],
                                      const unsigned char src1[4],
                                      const unsigned char src2[4]);
MINLINE void blend_color_darken_byte(unsigned char dst[4],
                                     const unsigned char src1[4],
                                     const unsigned char src2[4]);
MINLINE void blend_color_erase_alpha_byte(unsigned char dst[4],
                                          const unsigned char src1[4],
                                          const unsigned char src2[4]);
MINLINE void blend_color_add_alpha_byte(unsigned char dst[4],
                                        const unsigned char src1[4],
                                        const unsigned char src2[4]);

MINLINE void blend_color_overlay_byte(unsigned char dst[4],
                                      const uchar src1[4],
                                      const uchar src2[4]);
MINLINE void blend_color_hardlight_byte(unsigned char dst[4],
                                        const uchar src1[4],
                                        const uchar src2[4]);
MINLINE void blend_color_burn_byte(unsigned char dst[4], const uchar src1[4], const uchar src2[4]);
MINLINE void blend_color_linearburn_byte(unsigned char dst[4],
                                         const uchar src1[4],
                                         const uchar src2[4]);
MINLINE void blend_color_dodge_byte(unsigned char dst[4],
                                    const uchar src1[4],
                                    const uchar src2[4]);
MINLINE void blend_color_screen_byte(unsigned char dst[4],
                                     const uchar src1[4],
                                     const uchar src2[4]);
MINLINE void blend_color_softlight_byte(unsigned char dst[4],
                                        const uchar src1[4],
                                        const uchar src2[4]);
MINLINE void blend_color_pinlight_byte(unsigned char dst[4],
                                       const uchar src1[4],
                                       const uchar src2[4]);
MINLINE void blend_color_linearlight_byte(unsigned char dst[4],
                                          const uchar src1[4],
                                          const uchar src2[4]);
MINLINE void blend_color_vividlight_byte(unsigned char dst[4],
                                         const uchar src1[4],
                                         const uchar src2[4]);
MINLINE void blend_color_difference_byte(unsigned char dst[4],
                                         const uchar src1[4],
                                         const uchar src2[4]);
MINLINE void blend_color_exclusion_byte(unsigned char dst[4],
                                        const uchar src1[4],
                                        const uchar src2[4]);
MINLINE void blend_color_color_byte(unsigned char dst[4],
                                    const uchar src1[4],
                                    const uchar src2[4]);
MINLINE void blend_color_hue_byte(unsigned char dst[4], const uchar src1[4], const uchar src2[4]);
MINLINE void blend_color_saturation_byte(unsigned char dst[4],
                                         const uchar src1[4],
                                         const uchar src2[4]);
MINLINE void blend_color_luminosity_byte(unsigned char dst[4],
                                         const uchar src1[4],
                                         const uchar src2[4]);

MINLINE void blend_color_interpolate_byte(unsigned char dst[4],
                                          const unsigned char src1[4],
                                          const unsigned char src2[4],
                                          float t);

MINLINE void blend_color_mix_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_add_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_sub_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_mul_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_lighten_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_darken_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_erase_alpha_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_add_alpha_float(float dst[4], const float src1[4], const float src2[4]);

MINLINE void blend_color_overlay_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_hardlight_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_burn_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_linearburn_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_dodge_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_screen_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_softlight_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_pinlight_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_linearlight_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_vividlight_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_difference_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_exclusion_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_color_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_hue_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_saturation_float(float dst[4], const float src1[4], const float src2[4]);
MINLINE void blend_color_luminosity_float(float dst[4], const float src1[4], const float src2[4]);

MINLINE void blend_color_interpolate_float(float dst[4],
                                           const float src1[4],
                                           const float src2[4],
                                           float t);

#if KLI_MATH_DO_INLINE
#  include "intern/KLI_math_color_blend_inline.c"
#endif

#ifdef __cplusplus
}
#endif