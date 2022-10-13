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
#include "USD_scene_types.h"

#include "IMB_filetype.h"
#include "IMB_imbuf.h"

#include "IMB_colormanagement.h"

WABI_NAMESPACE_USING

int IMF_imtype_from_token(const wabi::TfToken &ftype)
{
  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_PNG) {
    return R_IMF_IMTYPE_PNG;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_TGA) {
    return R_IMF_IMTYPE_TARGA;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_RAWTGA) {
    return R_IMF_IMTYPE_RAWTGA;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_JPG) {
    return R_IMF_IMTYPE_JPEG90;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_JPEG) {
    return R_IMF_IMTYPE_JPEG90;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_BMP) {
    return R_IMF_IMTYPE_BMP;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_OPEN_EXR) {
    return R_IMF_IMTYPE_OPENEXR;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_IMAGIC) {
    return R_IMF_IMTYPE_MULTILAYER;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_PSD) {
    return R_IMF_IMTYPE_PSD;
  }
  
  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_JP2) {
    return R_IMF_IMTYPE_JP2;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_TIFF) {
    return R_IMF_IMTYPE_TIFF;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_TIF) {
    return R_IMF_IMTYPE_TIFF;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_HDR) {
    return R_IMF_IMTYPE_RADHDR;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_RADHDR) {
    return R_IMF_IMTYPE_RADHDR;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_CINEON) {
    return R_IMF_IMTYPE_CINEON;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_DPX) {
    return R_IMF_IMTYPE_DPX;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_DDS) {
    return R_IMF_IMTYPE_DDS;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_MPEG) {
    return R_IMF_IMTYPE_FFMPEG;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_WEBP) {
    return R_IMF_IMTYPE_WEBP;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_IRIS) {
    return R_IMF_IMTYPE_IRIS;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_IRIZ) {
    return R_IMF_IMTYPE_IRIZ;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_AVIRAW) {
    return R_IMF_IMTYPE_AVIRAW;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_AVIJPEG) {
    return R_IMF_IMTYPE_AVIJPEG;
  }

  /**
   * @NOTE: Some of these might come from plugins, so invalid
   * does not mean it's not supported. 
   * 
   * @REF: Check the #HioImageRegistry for all supported formats.
   */
  return R_IMF_IMTYPE_INVALID;
}

const wabi::TfToken IMF_imtype_dotext_from_token(const wabi::TfToken &ftype)
{
  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_PNG) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_PNG;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_TGA) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_TGA;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_RAWTGA) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_RAWTGA;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_JPG) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_JPG;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_JPEG) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_JPEG;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_BMP) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_BMP;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_OPEN_EXR) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_OPEN_EXR;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_IMAGIC) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_IMAGIC;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_PSD) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_PSD;
  }
  
  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_JP2) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_JP2;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_TIFF) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_TIFF;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_TIF) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_TIF;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_HDR) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_HDR;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_RADHDR) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_RADHDR;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_CINEON) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_CINEON;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_DPX) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_DPX;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_DDS) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_DDS;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_MPEG) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_MPEG;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_WEBP) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_WEBP;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_IRIS) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_IRIS;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_IRIZ) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_IRIZ;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_AVIRAW) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_AVIRAW;
  }

  if (ftype == ImbFileTypeToken::R_IMF_IMTYPE_TOKEN_AVIJPEG) {
    return ImbFileTypeDotExtToken::R_IMF_IMTYPE_DOT_EXT_TOKEN_AVIJPEG;
  }

  /**
   * @NOTE: Some of these might come from plugins, so invalid
   * does not mean it's not supported. 
   * 
   * @REF: Check the #HioImageRegistry for all supported formats.
   */
  return TfToken();
}
