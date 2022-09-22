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

#define MAX_COLORSPACE_NAME 64
#define MAX_COLORSPACE_DESCRIPTION 512

typedef struct ColorSpace
{
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

typedef struct ColorManagedView {
  int index;
  char name[MAX_COLORSPACE_NAME];
} ColorManagedView;

typedef struct ColorManagedLook {
  int index;
  char name[MAX_COLORSPACE_NAME];
  char ui_name[MAX_COLORSPACE_NAME];
  char view[MAX_COLORSPACE_NAME];
  char process_space[MAX_COLORSPACE_NAME];
  bool is_noop;
} ColorManagedLook;

void colormanagement_init(void);
void colormanagement_exit(void);

struct ColorManagedDisplay *colormanage_display_get_named(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* __KRAKEN_IMBUF_IMB_COLORMANAGEMENT_INTERN_H__ */