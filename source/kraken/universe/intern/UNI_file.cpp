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

#include "KLI_icons.h"
#include "KLI_string_utils.h"
#include "KLI_path_utils.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_report.h"
#include "KKE_screen.h"

#include "LUXO_runtime.h"

#include "UNI_api.h"
#include "UNI_area.h"
#include "UNI_context.h"
#include "UNI_default_tables.h"
#include "UNI_factory.h"
#include "UNI_file.h"
#include "UNI_scene.h"
#include "UNI_screen.h"
#include "UNI_userpref.h"
#include "UNI_window.h"
#include "UNI_workspace.h"

#include <wabi/wabi.h>

#include <wabi/base/vt/value.h>

#include <wabi/usd/usdUI/area.h>
#include <wabi/usd/usdUI/screen.h>
#include <wabi/usd/usdUI/tokens.h>
#include <wabi/usd/usdUI/window.h>
#include <wabi/usd/usdUI/workspace.h>

#include <wabi/usd/usdGeom/cube.h>
#include <wabi/usd/usdGeom/gprim.h>

#include <wabi/usd/usd/collectionAPI.h>
#include <wabi/usd/usd/stage.h>

WABI_NAMESPACE_BEGIN

static void decode_kraken_header(FileData *fd)
{
  char header[SIZEOFKRAKENHEADER], num[4];

  const std::string read = fd->sdf_handle->GetDocumentation();
  KLI_strncpy(header, CHARALL(read), SIZEOFKRAKENHEADER);

  if ((read.length() == sizeof(header)) && STREQLEN(header, "Kraken", 6) && (header[6] == char(" ")) &&
      (header[7] == char("v")) && (isdigit(header[8])) && (header[9] == char(".")) &&
      (isdigit(header[10])) && (isdigit(header[11])))
  {
    fd->flags |= FD_FLAGS_FILE_OK;

    /* get the version number */
    memcpy(num, CHARALL(STRINGALL(header[8]) + STRINGALL(header[10]) + STRINGALL(header[11])), 3);
    num[3] = 0;
    fd->fileversion = atoi(num);
  }
}

static FileData *kr_decode_and_check(FileData *fd, ReportList *reports)
{
  decode_kraken_header(fd);

  if (fd->flags & FD_FLAGS_FILE_OK)
  {
    const char *error_message = NULL;
    if (fd->sdf_handle->IsEmpty())
    {
      KKE_reportf(reports, RPT_ERROR, "Failed to read project file '%s': %s", fd->relabase, error_message);
      delete fd;
      fd = NULL;
    }
  } else
  {
    KKE_reportf(reports,
                RPT_ERROR,
                "Failed to read project file '%s', not a supported kraken file",
                fd->relabase);
    delete fd;
    fd = NULL;
  }

  return fd;
}

static FileData *filedata_new(KrakenFileReadReport *reports)
{
  KLI_assert(reports != NULL);

  FileData *fd = new FileData();

  fd->filedes = std::string();
  fd->reports = reports;

  return fd;
}

static FileData *kr_filedata_from_file_descriptor(const char *filepath,
                                                  KrakenFileReadReport *reports,
                                                  SdfLayerRefPtr file)
{
  char header[6];

  const std::string doc = file->GetDocumentation();
  auto filedoc = doc.substr(0, doc.find(' '));

  /* Regular file. */
  errno = 0;
  if (!STREQ(KLI_strncpy(header, CHARALL(filedoc), sizeof(header)), CHARALL(filedoc)))
  {
    KKE_reportf(reports->reports,
                RPT_WARNING,
                "Unable to read '%s': %s",
                filepath,
                errno ? strerror(errno) : TIP_("insufficient content"));
    return NULL;
  }

  if (!KLI_has_pixar_extension(file->GetFileExtension()))
  {
    KKE_reportf(reports->reports, RPT_WARNING, "Unrecognized file format '%s'", filepath);
    return NULL;
  }

  FileData *fd = filedata_new(reports);

  fd->filedes = file->GetIdentifier();
  fd->sdf_handle = SdfLayer::CreateNew(filepath);

  return fd;
}

static FileData *kr_filedata_from_file_open(const char *filepath, KrakenFileReadReport *reports)
{
  errno = 0;
  SdfLayerRefPtr file = SdfLayer::FindOrOpen(filepath);

  if (!file)
  {
    KKE_reportf(reports->reports,
                RPT_WARNING,
                "Unable to open '%s': %s",
                filepath,
                errno ? strerror(errno) : TIP_("unknown error reading file"));
    return NULL;
  }
  FileData *fd = kr_filedata_from_file_descriptor(filepath, reports, file);
  if ((fd == NULL) || (fd->filedes.empty()))
  {
    file->Clear();
  }
  return fd;
}

FileData *kr_filedata_from_file(const char *filepath, KrakenFileReadReport *reports)
{
  FileData *fd = kr_filedata_from_file_open(filepath, reports);
  if (fd != NULL)
  {
    /* needed for library_append and read_libraries */
    KLI_strncpy(fd->relabase, filepath, sizeof(fd->relabase));

    return kr_decode_and_check(fd, reports->reports);
  }
  return NULL;
}

KrakenHandle *KLO_krakenhandle_from_file(const char *filepath, KrakenFileReadReport *reports)
{
  KrakenHandle *kh;

  kh = (KrakenHandle *)kr_filedata_from_file(filepath, reports);

  return kh;
}

WABI_NAMESPACE_END