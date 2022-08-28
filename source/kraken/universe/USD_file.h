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
 * Universe.
 * Set the Stage.
 */

#include "USD_api.h"

KRAKEN_NAMESPACE_BEGIN

/**
 * In order of preferred
 * file extension, binary
 * Pixar files being the
 * most performant. */
enum eKrakenFileType
{
  KRAKEN_FILETYPE_USD = 1,
  KRAKEN_FILETYPE_USDC,
  KRAKEN_FILETYPE_USDZ,
  KRAKEN_FILETYPE_USDA,
};

struct KrakenFileData
{
  struct Main *main;
  struct UserDef *user;

  int fileflags;
  int globalf;
  char filename[FILE_MAX];

  struct kScreen *curscreen;
  struct Scene *curscene;

  eKrakenFileType type;
};

enum eUndoStepDir
{
  STEP_REDO = 1,
  STEP_UNDO = -1,
  STEP_INVALID = 0,
};

struct KrakenFileReadParams
{
  uint skip_flags : 3;
  uint is_startup : 1;

  eUndoStepDir undo_direction;
};

struct KrakenFileReadReport
{
  /* General reports handling. */
  struct ReportList *reports;
};

#define SIZEOFKRAKENHEADER 12

enum eFileDataFlag
{
  FD_FLAGS_FILE_OK = (1 << 3),
};

struct FileData
{
  int fileversion;

  /** #eFileDataFlag */
  int flags;

  char relabase[FILE_MAX];

  SdfLayerRefPtr sdf_handle;
  KrakenFileReadReport *reports;

  std::string filedes;
};

typedef FileData KrakenHandle;

FileData *kr_filedata_from_file(const char *filepath, KrakenFileReadReport *reports);

/**
 * Open a krakenhandle from a file path.
 *
 * @param filepath: The file path to open.
 * @param reports: Report errors in opening the file (can be NULL).
 * @return A handle on success, or NULL on failure. */
KrakenHandle *KLO_krakenhandle_from_file(const char *filepath, KrakenFileReadReport *reports);

KRAKEN_NAMESPACE_END