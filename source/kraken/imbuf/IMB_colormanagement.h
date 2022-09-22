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
struct EnumPropertyItem;
struct ImBuf;
struct ImageFormatData;
struct Main;
struct kContext;

struct ColorManagedDisplay;
struct ColorSpace;

/* -------------------------------------------------------------------- */
/** \name Display Functions
 * \{ */

const char *IMB_colormanagement_display_get_default_name(void);

/** \} */

#ifdef __cplusplus
}
#endif

#endif /* __KRAKEN_IMBUF_IMB_COLORMANAGEMENT_H__ */