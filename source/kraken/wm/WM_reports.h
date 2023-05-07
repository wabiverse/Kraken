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
 * Window Manager.
 * Making GUI Fly.
 */

typedef struct USDFileReadReport
{
  /* General reports handling. */
  struct ReportList *reports;

  /* Timing information. */
  struct
  {
    double whole;
    double libraries;
    double lib_overrides;
    double lib_overrides_resync;
    double lib_overrides_recursive_resync;
  } duration;

  /* Count information. */
  struct
  {
    /* Some numbers of IDs that ended up in a specific state, or required some specific process
     * during this file read. */
    int missing_libraries;
    int missing_linked_id;
    /* Some sub-categories of the above `missing_linked_id` counter. */
    int missing_obdata;
    int missing_obproxies;

    /* Number of root override IDs that were resynced. */
    int resynced_lib_overrides;

    /* Number of proxies converted to library overrides. */
    int proxies_to_lib_overrides_success;
    /* Number of proxies that failed to convert to library overrides. */
    int proxies_to_lib_overrides_failures;
    /* Number of sequencer strips that were not read because were in non-supported channels. */
    int sequence_strips_skipped;
  } count;

  /* Number of libraries which had overrides that needed to be resynced, and a single linked list
   * of those. */
  int resynced_lib_overrides_libraries_count;
  bool do_resynced_lib_overrides_libraries_list;
  struct LinkNode *resynced_lib_overrides_libraries;
} USDFileReadReport;
