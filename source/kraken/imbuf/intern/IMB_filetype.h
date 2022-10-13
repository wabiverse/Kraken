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

#pragma once

/**
 * @file
 * @ingroup IMBUF
 * Image Manipulation.
 */

#include "IMB_imbuf.h"

#include "USD_color_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------- */
/** @name Generic File Type
 * @{ */

struct ImBuf;

#define IM_FTYPE_FLOAT 1

typedef struct ImFileType
{
  /** Optional, called once when initializing. */
  void (*init)(void);
  /** Optional, called once when exiting. */
  void (*exit)(void);

  /**
   * Check if the data matches this file types 'magic',
   * @note that this may only read in a small part of the files header,
   * see: #IMB_ispic_type for details.
   */
  bool (*is_a)(const unsigned char *buf, size_t size);

  /** Load an image from memory. */
  struct ImBuf *(*load)(const unsigned char *mem,
                        size_t size,
                        int flags,
                        char colorspace[IM_MAX_SPACE]);
  /** Load an image from a file. */
  struct ImBuf *(*load_filepath)(const char *filepath, int flags, char colorspace[IM_MAX_SPACE]);
  /**
   * Load/Create a thumbnail image from a filepath. `max_thumb_size` is maximum size of either
   * dimension, so can return less on either or both. Should, if possible and performant, return
   * dimensions of the full-size image in r_width & r_height.
   */
  struct ImBuf *(*load_filepath_thumbnail)(const char *filepath,
                                           int flags,
                                           size_t max_thumb_size,
                                           char colorspace[IM_MAX_SPACE],
                                           size_t *r_width,
                                           size_t *r_height);
  /** Save to a file (or memory if #IB_mem is set in `flags` and the format supports it). */
  bool (*save)(struct ImBuf *ibuf, const char *filepath, int flags);
  void (*load_tile)(struct ImBuf *ibuf,
                    const unsigned char *mem,
                    size_t size,
                    int tx,
                    int ty,
                    unsigned int *rect);

  int flag;

  /** #eImbFileType */
  int filetype;

  int default_save_role;
} ImFileType;

extern const ImFileType IMB_FILE_TYPES[];
extern const ImFileType *IMB_FILE_TYPES_LAST;

const ImFileType *IMB_file_type_from_ftype(int ftype);
const ImFileType *IMB_file_type_from_ibuf(const ImBuf *ibuf);

/** @} */

#ifdef __cplusplus
}
#endif
