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

/**
 * @file
 * Universe.
 * Set the Stage.
 */

#pragma once

#include "UNI_api.h"
#include "UNI_scene.h"
#include "UNI_system.h"

/**
 * Global Universe Struct
 * - Holds pointer to the
 *   active Pixar Stage.
 * - Easy Scene data
 *   access.
 * - SysPaths & SysInfo. */

struct Universe {
  /** The active pixar stage. */
  wabi::UsdStageRefPtr stage;

  /** Quick access to scene data. */
  Scene scene;

  /** Covah syspaths and sysinfo. */
  System system;
};

/**
 * Covah SysPaths and SysInfo
 * - Directories relative to
 *   the covah installation.
 * - Simple access and set
 *   once for access to:
 *    - exe path
 *    - os tempdirs
 *    - covah themes
 *    - icons
 *    - plugins
 *    - textures
 *    - project file
 *    - build hash
 *    - version info */

COVAH_UNIVERSE_API
void UNI_create(std::string exe_path,
                std::string temp_dir,
                std::string styles_path,
                std::string icons_path,
                std::string datafiles_path,
                std::filesystem::path stage_id,
                uint64_t build_commit_timestamp,
                std::string build_hash,
                std::string covah_version);

/**
 * Pixar Stage IO
 * - Stage Creation.
 * - Stage Destruction
 * - Opening Stages.
 * - Saving Stages. */

COVAH_UNIVERSE_API
void UNI_create_stage(std::string project_file);

COVAH_UNIVERSE_API
void UNI_destroy(void);

COVAH_UNIVERSE_API
void UNI_open_stage(std::string project_file);

COVAH_UNIVERSE_API
void UNI_save_stage(void);

/**
 * Pixar Stage Access
 * - Obtain Pseudo Root. */

COVAH_UNIVERSE_API
const wabi::SdfPath &UNI_stage_root(void);

/**
 * Pixar Stage Defaults
 * - Covah GUI defaults.
 * - Covah Scene defaults. */

COVAH_UNIVERSE_API
void UNI_on_ctx(struct cContext *C);

COVAH_UNIVERSE_API
void UNI_author_default_scene(void);

/**
 * Global to Pixar Data */

COVAH_UNIVERSE_API
extern Universe UNI;
