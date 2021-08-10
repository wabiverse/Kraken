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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "KKE_utils.h"
#include "KKE_main.h"
#include "KKE_version.h"

WABI_NAMESPACE_BEGIN


std::string kraken_exe_path_init()
{
  return ArchGetExecutablePath();
}

std::string kraken_datafiles_path_init()
{
#ifdef _WIN32
  return STRCAT(G.main->exe_path, G.main->kraken_version_decimal + "/datafiles/");
#else
  /**
   * On Linux, datafiles directory lies outside of BIN
   * ex. BIN DATAFILES INCLUDE LIB PYTHON */
  return STRCAT(G.main->exe_path, "../datafiles/");
#endif
}

std::string kraken_python_path_init()
{
#ifdef _WIN32
  return STRCAT(G.main->exe_path, G.main->kraken_version_decimal + "/python/lib/");
#else
  return STRCAT(G.main->exe_path, "../python/lib/python3.9/site-packages");
#endif
}

std::string kraken_fonts_path_init()
{
#ifdef _WIN32
  return STRCAT(G.main->exe_path, G.main->kraken_version_decimal + "/datafiles/fonts/");
#else
  return STRCAT(G.main->exe_path, "../datafiles/fonts/");
#endif
}

std::string kraken_icon_path_init()
{
#ifdef _WIN32
  return STRCAT(G.main->exe_path, G.main->kraken_version_decimal + "/datafiles/icons/");
#else
  return STRCAT(G.main->exe_path, "../datafiles/icons/");
#endif
}

std::string kraken_startup_file_init()
{
#ifdef _WIN32
  return STRCAT(G.main->exe_path, G.main->kraken_version_decimal + "/datafiles/startup.usd");
#else
  return STRCAT(G.main->exe_path, "../datafiles/startup.usd");
#endif
}

std::string kraken_system_tempdir_path()
{
  return std::filesystem::temp_directory_path().string();
}


WABI_NAMESPACE_END