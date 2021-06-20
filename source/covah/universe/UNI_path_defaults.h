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

#pragma once

/**
 * @file
 * Universe.
 * Set the Stage.
 */

#include <wabi/base/tf/token.h>


WABI_NAMESPACE_BEGIN

/**
 * These are the default Covah data  structure  locations
 * relative to a Stage's Root Path. Hardcoded - well and
 * defined in one place - so we can ensure to maintain
 * compatibility  with  all future versions to come. We
 * always need to know where to find our own core  data
 * structures so we can be certain that we do not corrupt
 * older user project files with new versions of Covah.
 *
 * !! IF YOU ADD A NEW PATH.  INCLUDE IT HERE. */


struct COVAH_PATH_DEFAULTS
{
  /** Covah Root Namespace. */
  const inline static TfToken COVAH_ROOT = TfToken("/Covah");

  /** Covah :: Ops Root. */
  const inline static TfToken COVAH_OPERATORS = TfToken("/Covah/Ops");

  /** Covah :: UserDef Root. */
  const inline static TfToken COVAH_USERPREFS = TfToken("/Covah/UserDef");

  /** Covah :: MainWindow Root. */
  const inline static TfToken COVAH_WINDOW = TfToken("/Covah/MainWindow");

  /** Covah :: MainWindow :: Workspaces Root. */
  const inline static TfToken COVAH_WORKSPACES = TfToken("/Covah/MainWindow/Workspaces");

  /** Covah :: MainWindow :: Workspaces :: Layout Root. */
  const inline static TfToken COVAH_WORKSPACES_LAYOUT = TfToken("/Covah/MainWindow/Workspaces/Layout");

  /** Covah :: MainWindow :: Workspaces :: Layout :: Screen Root. */
  const inline static TfToken COVAH_SCREEN_LAYOUT = TfToken("/Covah/MainWindow/Workspaces/Layout/Screen");
};

WABI_NAMESPACE_END
