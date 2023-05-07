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
 * Derived from original work by Copyright 2022, Blender Foundation.
 * From the Blender Library. (source/blender/blenlib).
 *
 * With any additions or modifications specific to Kraken.
 *
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#pragma once

/**
 * @file
 * Kraken Loader.
 * USD & co.
 */

#include "KLI_listbase.h"
#include "KLI_sys_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct KrakenThumbnail;

struct KrakenThumbnail *KLO_thumbnail_from_file(const char *filepath);

void KLO_update_defaults_startup_usd(struct Main *kmain, const char *app_template);
void KLO_update_defaults_workspace(struct WorkSpace *workspace, const char *app_template);


/* skip reading some data-block types (may want to skip screen data too). */
typedef enum eKLOReadSkip
{
  KLO_READ_SKIP_NONE = 0,
  KLO_READ_SKIP_USERDEF = (1 << 0),
  KLO_READ_SKIP_DATA = (1 << 1),
  /** Do not attempt to re-use IDs from old bmain for unchanged ones in case of undo. */
  KLO_READ_SKIP_UNDO_OLD_MAIN = (1 << 2),
} eKLOReadSkip;
#define KLO_READ_SKIP_ALL (KLO_READ_SKIP_USERDEF | KLO_READ_SKIP_DATA)

#ifdef __cplusplus
inline eKLOReadSkip operator |(eKLOReadSkip a, eKLOReadSkip b)
{
  return static_cast<eKLOReadSkip>(static_cast<int>(a) | static_cast<int>(b));
}

inline eKLOReadSkip operator &(eKLOReadSkip a, eKLOReadSkip b)
{
  return static_cast<eKLOReadSkip>(static_cast<int>(a) & static_cast<int>(b));
}

inline eKLOReadSkip& operator |=(eKLOReadSkip& a, eKLOReadSkip b)
{
  return a = a | b;
}
#endif /* __cplusplus */


struct USDFileData *KLO_read_from_memory(const void *mem,
                                         int memsize,
                                         eKLOReadSkip skip_flags,
                                         struct ReportList *reports);


/* datafiles (generated theme) */
extern const struct kTheme U_theme_default;
extern const struct UserDef U_default;

#ifdef __cplusplus
}
#endif
