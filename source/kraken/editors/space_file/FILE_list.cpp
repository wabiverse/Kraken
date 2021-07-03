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
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * Editors.
 * Tools for Artists.
 */

#include "ED_fileselect.h"

#include "KLI_icons.h"
#include "KLI_path_utils.h"

#include "UNI_space_types.h"

WABI_NAMESPACE_BEGIN

/* would recognize (.usd, .usda, .usdc, .usdz) as well */
static bool file_is_pixar_backup(const char *str)
{
  const size_t a = strlen(str);

  size_t usd = 5;
  if (a == 0 || usd >= a)
  {
    /* pass */
  }
  else
  {
    const char *locusd;

    if (a > usd + 1)
    {
      usd++;
    }

    /* allow .usd1 .usd2 .usd32 */
    locusd = KLI_strcasestr(str + a - usd, ".usd");
    if (locusd)
    {
      return true;
    }
  }

  size_t usdx = 6;
  if (a == 0 || usdx >= a)
  {
    /* pass */
  }
  else
  {
    const char *locusdx;

    if (a > usdx + 1)
    {
      usdx++;
    }

    /* allow .usda1 .usda2 .usdc3 .usdz32 */
    locusdx = KLI_strcasestr(str + a - usdx, ".usda");
    if (locusdx)
    {
      return true;
    }
    locusdx = KLI_strcasestr(str + a - usdx, ".usdc");
    if (locusdx)
    {
      return true;
    }
    locusdx = KLI_strcasestr(str + a - usdx, ".usdz");
    if (locusdx)
    {
      return true;
    }
  }

  return false;
}

int ED_path_extension_type(const std::string &path)
{

  if (KLI_has_pixar_extension(path))
  {
    return FILE_TYPE_PIXAR;
  }


  if (file_is_pixar_backup(CHARALL(path)))
  {
    return FILE_TYPE_PIXAR_BACKUP;
  }


  if (KLI_path_extension_check(CHARALL(path), ".app"))
  {
    return FILE_TYPE_APPLICATIONBUNDLE;
  }


  if (KLI_path_extension_check(CHARALL(path), ".py"))
  {
    return FILE_TYPE_PYSCRIPT;
  }


  if (KLI_path_extension_check_n(CHARALL(path),
                                 ".txt",
                                 ".glsl",
                                 ".osl",
                                 ".data",
                                 ".pov",
                                 ".ini",
                                 ".mcr",
                                 ".inc",
                                 ".fountain",
                                 NULL))
  {
    return FILE_TYPE_TEXT;
  }


  if (KLI_path_extension_check_n(CHARALL(path), ".ttf", ".ttc", ".pfb", ".otf", ".otc", NULL))
  {
    return FILE_TYPE_FTFONT;
  }


  if (KLI_path_extension_check(CHARALL(path), ".btx"))
  {
    return FILE_TYPE_BTX;
  }


  if (KLI_path_extension_check(CHARALL(path), ".dae"))
  {
    return FILE_TYPE_COLLADA;
  }


  if (KLI_path_extension_check(CHARALL(path), ".abc"))
  {
    return FILE_TYPE_ALEMBIC;
  }


  if (KLI_path_extension_check_n(CHARALL(path), ".blend", NULL))
  {
    return FILE_TYPE_BLENDER;
  }


  if (KLI_path_extension_check(CHARALL(path), ".vdb"))
  {
    return FILE_TYPE_VOLUME;
  }


  if (KLI_path_extension_check(CHARALL(path), ".zip"))
  {
    return FILE_TYPE_ARCHIVE;
  }


  if (KLI_path_extension_check_n(CHARALL(path), ".obj", ".3ds", ".fbx", ".glb", ".gltf", ".svg", NULL))
  {
    return FILE_TYPE_OBJECT_IO;
  }


  if (KLI_path_extension_check_n(CHARALL(path),
                                 ".png",
                                 ".tga",
                                 ".bmp",
                                 ".jpg",
                                 ".jpeg",
                                 ".sgi",
                                 ".rgb",
                                 ".rgba",
                                 ".tif",
                                 ".tiff",
                                 ".tx",
                                 ".jp2",
                                 ".j2c",
                                 ".hdr",
                                 ".dds",
                                 ".dpx",
                                 ".cin",
                                 ".exr",
                                 ".psd",
                                 ".pdd",
                                 ".psb",
                                 NULL))
  {
    return FILE_TYPE_IMAGE;
  }


  if (KLI_path_extension_check_n(CHARALL(path),
                                 ".avi",
                                 ".flc",
                                 ".mov",
                                 ".movie",
                                 ".mp4",
                                 ".m4v",
                                 ".m2v",
                                 ".m2t",
                                 ".m2ts",
                                 ".mts",
                                 ".ts",
                                 ".mv",
                                 ".avs",
                                 ".wmv",
                                 ".ogv",
                                 ".ogg",
                                 ".r3d",
                                 ".dv",
                                 ".mpeg",
                                 ".mpg",
                                 ".mpg2",
                                 ".vob",
                                 ".mkv",
                                 ".flv",
                                 ".divx",
                                 ".xvid",
                                 ".mxf",
                                 ".webm",
                                 NULL))
  {
    return FILE_TYPE_MOVIE;
  }


  if (KLI_path_extension_check_n(CHARALL(path),
                                 ".wav",
                                 ".ogg",
                                 ".oga",
                                 ".mp3",
                                 ".mp2",
                                 ".ac3",
                                 ".aac",
                                 ".flac",
                                 ".wma",
                                 ".eac3",
                                 ".aif",
                                 ".aiff",
                                 ".m4a",
                                 ".mka",
                                 NULL))
  {
    return FILE_TYPE_SOUND;
  }


  return 0;
}

int ED_file_extension_icon(const std::string &path)
{
  const int type = ED_path_extension_type(path);

  switch (type)
  {
    case FILE_TYPE_PIXAR:
      return ICON_KRAKEN;

    case FILE_TYPE_PIXAR_BACKUP:
      return ICON_KRAKEN;

    case FILE_TYPE_IMAGE:
      return ICON_IMAGING;

    case FILE_TYPE_MOVIE:
      return ICON_ANIMATION;

    case FILE_TYPE_PYSCRIPT:
      return ICON_SCRIPTING;

    case FILE_TYPE_SOUND:
      return ICON_GRID;

    case FILE_TYPE_FTFONT:
      return ICON_GRID;

    case FILE_TYPE_BTX:
      return ICON_GRID;

    case FILE_TYPE_COLLADA:
    case FILE_TYPE_ALEMBIC:
    case FILE_TYPE_OBJECT_IO:
      return ICON_GEOM;

    case FILE_TYPE_TEXT:
      return ICON_GRID;

    case FILE_TYPE_ARCHIVE:
      return ICON_GRID;

    case FILE_TYPE_VOLUME:
      return ICON_SIMULATION;

    default:
      return ICON_GRID;
  }
}

WABI_NAMESPACE_END