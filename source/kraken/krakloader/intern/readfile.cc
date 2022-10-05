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

/**
 * @file
 * Kraken Loader.
 * USD & co.
 */

#include "MEM_guardedalloc.h"

#include "KKE_main.h"
#include "USD_file.h"

#include "KLI_string.h"

#include "KLO_readfile.h"

#include <wabi/usd/usd/stage.h>

WABI_NAMESPACE_USING

static int *read_file_thumbnail(FileData *fd)
{
  int *kraken_thumb = nullptr;

  /* clang-format off */
  int data[5][5] = { 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0 };
  /* clang-format on */

  kraken_thumb = &data[5][5];

  return kraken_thumb;
}

KrakenThumbnail *KLO_thumbnail_from_file(const char *filepath)
{
  FileData *fd;
  KrakenThumbnail *data = nullptr;
  int *fd_data;

  fd = new FileData();
  fd->filedes = KLI_strdup("");
  fd->sdf_handle = SdfLayer::FindOrOpen(filepath);
  fd_data = fd->sdf_handle ? read_file_thumbnail(fd) : nullptr;

  if (fd_data) {
    data = MEM_cnew<KrakenThumbnail>("KrakenThumbnail");
    data->width = 5;
    data->height = 5;

    size_t data_size = (sizeof(KrakenThumbnail) +
                        ((size_t)(data->width) * (size_t)(data->height)) * sizeof(int));
    memcpy(data->rect, &fd_data[2], data_size - sizeof(*data));
  }

  if (fd) {
    fd->sdf_handle.Reset();
    delete fd;
  }

  return data;
}