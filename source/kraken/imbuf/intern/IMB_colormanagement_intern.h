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

#ifndef __KRAKEN_IMBUF_IMB_COLORMANAGEMENT_INTERN_H__
#define __KRAKEN_IMBUF_IMB_COLORMANAGEMENT_INTERN_H__

/**
 * @file
 * @ingroup IMBUF
 * Image Manipulation.
 */

#include "KLI_sys_types.h"
#include "USD_listBase.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ImBuf;
struct OCIO_ConstCPUProcessorRcPtr;

extern float imbuf_luma_coefficients[3];
extern float imbuf_scene_linear_to_xyz[3][3];
extern float imbuf_xyz_to_scene_linear[3][3];
extern float imbuf_scene_linear_to_aces[3][3];
extern float imbuf_aces_to_scene_linear[3][3];
extern float imbuf_scene_linear_to_rec709[3][3];
extern float imbuf_rec709_to_scene_linear[3][3];

#define MAX_COLORSPACE_NAME 64
#define MAX_COLORSPACE_DESCRIPTION 512

typedef struct ColorSpace
{
  struct ColorSpace *next, *prev;
  int index;
  char name[MAX_COLORSPACE_NAME];
  char description[MAX_COLORSPACE_DESCRIPTION];

  struct OCIO_ConstCPUProcessorRcPtr *to_scene_linear;
  struct OCIO_ConstCPUProcessorRcPtr *from_scene_linear;

  char (*aliases)[MAX_COLORSPACE_NAME];
  int num_aliases;

  bool is_invertible;
  bool is_data;

  /* Additional info computed only when needed since it's not cheap. */
  struct
  {
    bool cached;
    bool is_srgb;
    bool is_scene_linear;
  } info;
} ColorSpace;

typedef struct ColorManagedDisplay
{
  struct ColorManagedDisplay *next, *prev;
  int index;
  char name[MAX_COLORSPACE_NAME];
  ListBase views; /* LinkData.data -> ColorManagedView */

  struct OCIO_ConstCPUProcessorRcPtr *to_scene_linear;
  struct OCIO_ConstCPUProcessorRcPtr *from_scene_linear;
} ColorManagedDisplay;

typedef struct ColorManagedView
{
  struct ColorManagedView *next, *prev;
  int index;
  char name[MAX_COLORSPACE_NAME];
} ColorManagedView;

typedef struct ColorManagedLook
{
  struct ColorManagedLook *next, *prev;
  int index;
  char name[MAX_COLORSPACE_NAME];
  char ui_name[MAX_COLORSPACE_NAME];
  char view[MAX_COLORSPACE_NAME];
  char process_space[MAX_COLORSPACE_NAME];
  bool is_noop;
} ColorManagedLook;

void colormanagement_init(void);
void colormanagement_exit(void);

const char *colormanage_display_get_default_name(void);
struct ColorManagedDisplay *colormanage_display_get_default(void);
struct ColorManagedDisplay *colormanage_display_add(const char *name);
struct ColorManagedDisplay *colormanage_display_get_named(const char *name);
struct ColorManagedDisplay *colormanage_display_get_indexed(int index);

const char *colormanage_view_get_default_name(const ColorManagedDisplay *display);
struct ColorManagedView *colormanage_view_get_default(const ColorManagedDisplay *display);
struct ColorManagedView *colormanage_view_add(const char *name);
struct ColorManagedView *colormanage_view_get_indexed(int index);
struct ColorManagedView *colormanage_view_get_named(const char *name);
struct ColorManagedView *colormanage_view_get_named_for_display(const char *display_name,
                                                                const char *name);

struct ColorSpace *colormanage_colorspace_add(const char *name,
                                              const char *description,
                                              bool is_invertible,
                                              bool is_data);
struct ColorSpace *colormanage_colorspace_get_named(const char *name);
struct ColorSpace *colormanage_colorspace_get_roled(int role);
struct ColorSpace *colormanage_colorspace_get_indexed(int index);

struct ColorManagedLook *colormanage_look_add(const char *name,
                                              const char *process_space,
                                              bool is_noop);
struct ColorManagedLook *colormanage_look_get_named(const char *name);
struct ColorManagedLook *colormanage_look_get_indexed(int index);


/* -------------------------------------------------------------------- */
/** @name Imbuf Color Management Flag
 *
 * @brief Used with #ImBuf.colormanage_flag
 * @{ */

enum
{
  IMB_COLORMANAGE_IS_DATA = (1 << 0),
};

typedef enum eImBufFlags
{
  IB_rect = 1 << 0,
  IB_test = 1 << 1,
  IB_zbuf = 1 << 3,
  IB_mem = 1 << 4,
  IB_rectfloat = 1 << 5,
  IB_zbuffloat = 1 << 6,
  IB_multilayer = 1 << 7,
  IB_metadata = 1 << 8,
  IB_animdeinterlace = 1 << 9,
  IB_tiles = 1 << 10,
  IB_tilecache = 1 << 11,
  /** indicates whether image on disk have premul alpha */
  IB_alphamode_premul = 1 << 12,
  /** if this flag is set, alpha mode would be guessed from file */
  IB_alphamode_detect = 1 << 13,
  /* alpha channel is unrelated to RGB and should not affect it */
  IB_alphamode_channel_packed = 1 << 14,
  /** ignore alpha on load and substitute it with 1.0f */
  IB_alphamode_ignore = 1 << 15,
  IB_thumbnail = 1 << 16,
  IB_multiview = 1 << 17,
  IB_halffloat = 1 << 18,
} eImBufFlags;

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __KRAKEN_IMBUF_IMB_COLORMANAGEMENT_INTERN_H__ */