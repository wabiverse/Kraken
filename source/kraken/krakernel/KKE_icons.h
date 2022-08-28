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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#pragma once

#include "KKE_api.h"
#include "KKE_context.h"

#include <wabi/base/tf/token.h>

KRAKEN_NAMESPACE_BEGIN

typedef struct PreviewImage
{
  /* All values of 2 are really NUM_ICON_SIZES */
  GfVec2i w;
  GfVec2i h;
  GfVec2i flag;
  GfVec2i changed_timestamp;
  GfVec2i *rect;

  TfToken icon_id;

  /** Runtime data. */
  short tag;
} PreviewImage;

typedef void (*DrawInfoFreeFP)(void *drawinfo);

enum eCKEIconTypes
{
  ICON_DATA_ID = 0,
  ICON_DATA_IMBUF,
  ICON_DATA_PREVIEW,
  ICON_DATA_GEOM,
  ICON_DATA_STUDIOLIGHT
};

struct Icon
{
  void *drawinfo;
  void *obj;
  char obj_type;
  /** Internal use only. */
  char flag;
  /** #ID_Type or 0 when not used for ID preview. */
  short id_type;
  DrawInfoFreeFP drawinfo_free;
};

void KKE_icon_changed(const int icon_id);

KRAKEN_NAMESPACE_END