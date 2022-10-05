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

#ifdef __cplusplus
extern "C" {
#endif

void KLI_bicubic_interpolation_fl(
    const float *buffer, float *output, int width, int height, int components, float u, float v);

void KLI_bicubic_interpolation_char(const unsigned char *buffer,
                                    unsigned char *output,
                                    int width,
                                    int height,
                                    int components,
                                    float u,
                                    float v);

void KLI_bilinear_interpolation_fl(
    const float *buffer, float *output, int width, int height, int components, float u, float v);

void KLI_bilinear_interpolation_char(const unsigned char *buffer,
                                     unsigned char *output,
                                     int width,
                                     int height,
                                     int components,
                                     float u,
                                     float v);

void KLI_bilinear_interpolation_wrap_fl(const float *buffer,
                                        float *output,
                                        int width,
                                        int height,
                                        int components,
                                        float u,
                                        float v,
                                        bool wrap_x,
                                        bool wrap_y);

void KLI_bilinear_interpolation_wrap_char(const unsigned char *buffer,
                                          unsigned char *output,
                                          int width,
                                          int height,
                                          int components,
                                          float u,
                                          float v,
                                          bool wrap_x,
                                          bool wrap_y);

#define EWA_MAXIDX 255
extern const float EWA_WTS[EWA_MAXIDX + 1];

typedef void (*ewa_filter_read_pixel_cb)(void *userdata, int x, int y, float result[4]);

void KLI_ewa_imp2radangle(
    float A, float B, float C, float F, float *a, float *b, float *th, float *ecc);

/**
 * TODO(sergey): Consider making this function inlined, so the pixel read callback
 * could also be inlined in order to avoid per-pixel function calls.
 */
void KLI_ewa_filter(int width,
                    int height,
                    bool intpol,
                    bool use_alpha,
                    const float uv[2],
                    const float du[2],
                    const float dv[2],
                    ewa_filter_read_pixel_cb read_pixel_cb,
                    void *userdata,
                    float result[4]);

#ifdef __cplusplus
}
#endif
