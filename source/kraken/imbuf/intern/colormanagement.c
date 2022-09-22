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

#include "IMB_colormanagement.h"
#include "IMB_colormanagement_intern.h"

#include <math.h>
#include <string.h>

#include <ocio_capi.h>

/* -------------------------------------------------------------------- */
/** \name Global declarations
 * \{ */

#define DISPLAY_BUFFER_CHANNELS 4

static ListBase global_displays = {NULL, NULL};

/** \} */

/* -------------------------------------------------------------------- */
/** \name Display Functions
 * \{ */

const char *colormanage_display_get_default_name(void)
{
  OCIO_ConstConfigRcPtr *config = OCIO_getCurrentConfig();
  const char *display_name;

  display_name = OCIO_configGetDefaultDisplay(config);

  OCIO_configRelease(config);

  return display_name;
}

ColorManagedDisplay *colormanage_display_get_default(void)
{
  const char *display_name = colormanage_display_get_default_name();

  if (display_name[0] == '\0') {
    return NULL;
  }

  return colormanage_display_get_named(display_name);
}

ColorManagedDisplay *colormanage_display_get_named(const char *name)
{
  ColorManagedDisplay *display;

  for (display = global_displays.first; display; display = display->next) {
    if (STREQ(display->name, name)) {
      return display;
    }
  }

  return NULL;
}


const char *IMB_colormanagement_display_get_default_name(void)
{
  ColorManagedDisplay *display = colormanage_display_get_default();

  return display->name;
}

/** \} */
