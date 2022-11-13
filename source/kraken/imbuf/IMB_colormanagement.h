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

#ifndef __KRAKEN_IMBUF_IMB_COLORMANAGEMENT_H__
#define __KRAKEN_IMBUF_IMB_COLORMANAGEMENT_H__

/**
 * @file
 * @ingroup IMBUF
 * Image Manipulation.
 */

#include "KLI_compiler_compat.h"
#include "KLI_sys_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KCM_CONFIG_FILE "config.ocio"

struct ColorManagedColorspaceSettings;
struct ColorManagedDisplaySettings;
struct ColorManagedViewSettings;
struct ColorManagedOutputSettings;
struct ColormanageProcessor;
struct EnumPROP;
struct ImBuf;
struct ImageFormatData;
struct Main;
struct kContext;

struct ColorManagedDisplay;
struct ColorSpace;

/* -------------------------------------------------------------------- */
/** @name Generic Functions
 * @{ */

void IMB_colormanagement_check_file_config(struct Main *kmain);

const char *IMB_colormanagement_role_colorspace_name_get(int role);

/** @} */

/* -------------------------------------------------------------------- */
/** @name Display Functions
 * @{ */

int IMB_colormanagement_display_get_named_index(const char *name);
const char *IMB_colormanagement_display_get_indexed_name(int index);
const char *IMB_colormanagement_display_get_default_name(void);

struct ColorManagedDisplay *IMB_colormanagement_display_get_named(const char *name);
const char *IMB_colormanagement_display_get_none_name(void);
const char *IMB_colormanagement_display_get_default_view_transform_name(
  struct ColorManagedDisplay *display);

/** @} */

/**
 * @note Same as IMB_colormanagement_setup_glsl_draw,
 * but display space conversion happens from a specified space.
 *
 * Configures GLSL shader for conversion from specified to
 * display color space
 *
 * Will create appropriate OCIO processor and setup GLSL shader,
 * so further 2D texture usage will use this conversion.
 *
 * When there's no need to apply transform on 2D textures, use
 * IMB_colormanagement_finish_glsl_draw().
 *
 * This is low-level function, use ED_draw_imbuf_ctx if you
 * only need to display given image buffer
 */
bool IMB_colormanagement_setup_glsl_draw_from_space(
  const struct ColorManagedViewSettings *view_settings,
  const struct ColorManagedDisplaySettings *display_settings,
  struct ColorSpace *colorspace,
  float dither,
  bool predivide,
  bool do_overlay_merge);

/**
 * Same as #IMB_colormanagement_scene_linear_to_display_v3,
 * but converts color in opposite direction.
 */
void IMB_colormanagement_display_to_scene_linear_v3(float pixel[3],
                                                    struct ColorManagedDisplay *display);
void IMB_colormanagement_colorspace_to_scene_linear_v4(float pixel[4],
                                                       bool predivide,
                                                       struct ColorSpace *colorspace);
/**
 * Finish GLSL-based display space conversion.
 */
void IMB_colormanagement_finish_glsl_draw(void);

/** @} */

/* -------------------------------------------------------------------- */
/** \name View Transform
 * \{ */

/* Roles */
enum
{
  COLOR_ROLE_SCENE_LINEAR = 0,
  COLOR_ROLE_COLOR_PICKING,
  COLOR_ROLE_TEXTURE_PAINTING,
  COLOR_ROLE_DEFAULT_SEQUENCER,
  COLOR_ROLE_DEFAULT_BYTE,
  COLOR_ROLE_DEFAULT_FLOAT,
  COLOR_ROLE_DATA,
};

/** \} */

#ifdef __cplusplus
}
#endif

#endif /* __KRAKEN_IMBUF_IMB_COLORMANAGEMENT_H__ */