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

/** Covah Root Namespace. */
#define COVAH_ROOT / Covah

/** Covah :: MainWindow Root. */
#define COVAH_WINDOW COVAH_ROOT / MainWindow

/** Covah :: MainWindow :: Workspaces Root. */
#define COVAH_WORKSPACES COVAH_WINDOW / Workspaces

/** Covah :: MainWindow :: Workspaces :: Layout Root. */
#define COVAH_WORKSPACES_LAYOUT COVAH_WORKSPACES / Layout

/** Covah :: MainWindow :: Workspaces :: Layout :: Screen Root. */
#define COVAH_SCREEN_LAYOUT COVAH_WORKSPACES_LAYOUT / Screen