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

#include <stddef.h>

#include "KLI_utildefines.h"

#include "USD_color_types.h"

#include "IMB_filetype.h"
#include "IMB_imbuf.h"
// #include "IMB_imbuf_types.h"

#include "IMB_colormanagement.h"

const ImFileType IMB_FILE_TYPES[] = {
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = 0,
    .filetype = IMB_FTYPE_JPG,
    .default_save_role = COLOR_ROLE_DEFAULT_BYTE,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = 0,
    .filetype = IMB_FTYPE_PNG,
    .default_save_role = COLOR_ROLE_DEFAULT_BYTE,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = 0,
    .filetype = IMB_FTYPE_BMP,
    .default_save_role = COLOR_ROLE_DEFAULT_BYTE,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = 0,
    .filetype = IMB_FTYPE_TGA,
    .default_save_role = COLOR_ROLE_DEFAULT_BYTE,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = 0,
    .filetype = IMB_FTYPE_IMAGIC,
    .default_save_role = COLOR_ROLE_DEFAULT_BYTE,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = IM_FTYPE_FLOAT,
    .filetype = IMB_FTYPE_DPX,
    .default_save_role = COLOR_ROLE_DEFAULT_FLOAT,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = IM_FTYPE_FLOAT,
    .filetype = IMB_FTYPE_CINEON,
    .default_save_role = COLOR_ROLE_DEFAULT_FLOAT,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = 0,
    .filetype = IMB_FTYPE_TIF,
    .default_save_role = COLOR_ROLE_DEFAULT_BYTE,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = IM_FTYPE_FLOAT,
    .filetype = IMB_FTYPE_RADHDR,
    .default_save_role = COLOR_ROLE_DEFAULT_FLOAT,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = IM_FTYPE_FLOAT,
    .filetype = IMB_FTYPE_OPENEXR,
    .default_save_role = COLOR_ROLE_DEFAULT_FLOAT,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = IM_FTYPE_FLOAT,
    .filetype = IMB_FTYPE_JP2,
    .default_save_role = COLOR_ROLE_DEFAULT_BYTE,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = IM_FTYPE_FLOAT,
    .filetype = IMB_FTYPE_PSD,
    .default_save_role = COLOR_ROLE_DEFAULT_FLOAT,
  },
  {
    .init = NULL,
    .exit = NULL,
    .is_a = NULL,
    .load = NULL,
    .load_filepath = NULL,
    .load_filepath_thumbnail = NULL,
    .save = NULL,
    .load_tile = NULL,
    .flag = 0,
    .filetype = IMB_FTYPE_WEBP,
    .default_save_role = COLOR_ROLE_DEFAULT_BYTE,
  },
  {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0},
};

const ImFileType *IMB_FILE_TYPES_LAST = &IMB_FILE_TYPES[ARRAY_SIZE(IMB_FILE_TYPES) - 1];

const ImFileType *IMB_file_type_from_ftype(int ftype)
{
  for (const ImFileType *type = IMB_FILE_TYPES; type < IMB_FILE_TYPES_LAST; type++) {
    if (ftype == type->filetype) {
      return type;
    }
  }
  return NULL;
}

const ImFileType *IMB_file_type_from_ibuf(const ImBuf *ibuf)
{
  return IMB_file_type_from_ftype(ibuf->ftype);
}