/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2009 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup krf
 *
 * Default API, that uses Blender's user preferences for the default size.
 */

#include "USD_userpref.h"

#include "KLI_assert.h"

#include "KRF_api.h"

#include "krf_internal.h"

/* call KRF_default_set first! */
#define ASSERT_DEFAULT_SET KLI_assert(global_font_default != -1)

/* Default size and dpi, for KRF_draw_default. */
static int global_font_default = -1;
static int global_font_dpi = 72;
/* Keep in sync with `UI_DEFAULT_TEXT_POINTS` */
static float global_font_size = 11.0f;

void KRF_default_dpi(int dpi)
{
  global_font_dpi = dpi;
}

void KRF_default_size(float size)
{
  global_font_size = size;
}

void KRF_default_set(int fontid)
{
  if ((fontid == -1) || krf_font_id_is_valid(fontid)) {
    global_font_default = fontid;
  }
}

int KRF_default(void)
{
  ASSERT_DEFAULT_SET;
  return global_font_default;
}

int KRF_set_default(void)
{
  ASSERT_DEFAULT_SET;

  KRF_size(global_font_default, global_font_size, global_font_dpi);

  return global_font_default;
}

void KRF_draw_default(float x, float y, float z, const char *str, const size_t str_len)
{
  ASSERT_DEFAULT_SET;
  KRF_size(global_font_default, global_font_size, global_font_dpi);
  KRF_position(global_font_default, x, y, z);
  KRF_draw(global_font_default, str, str_len);
}
