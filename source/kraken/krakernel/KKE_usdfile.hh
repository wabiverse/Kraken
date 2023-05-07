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
 * KRAKEN Kernel.
 * Purple Underground.
 */

void KKE_usdfile_read_setup(struct kContext *C,
                            struct USDFileData *usd,
                            const struct USDFileReadParams *params,
                            struct USDFileReadReport *reports);

struct USDFileData *KKE_usdfile_read(const char *filepath,
                                     const struct USDFileReadParams *params,
                                     struct USDFileReadReport *reports);

struct UserDef *KKE_usdfile_userdef_from_defaults(void);

void KKE_usdfile_read_setup_ex(struct kContext *C,
                               struct USDFileData *usd,
                               const struct USDFileReadParams *params,
                               struct USDFileReadReport *reports,
                               /* Extra args. */
                               const bool startup_update_defaults,
                               const char *startup_app_template);

struct USDFileData *KKE_usdfile_read_from_memory(const void *filebuf,
                                                 int filelength,
                                                 const struct USDFileReadParams *params,
                                                 struct ReportList *reports);
