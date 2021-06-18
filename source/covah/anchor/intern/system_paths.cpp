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
 * Anchor.
 * Bare Metal.
 */

#include "ANCHOR_system_paths.h"

#include <wabi/base/arch/systemInfo.h>

WABI_NAMESPACE_USING

#include <stdio.h>

#if defined(__linux__)
#  include <sstream>

#  include <sys/time.h>
#  include <unistd.h>

#  include <cstdlib>
#  include <stdio.h>

#  include <pwd.h>
#  include <string>

using std::string;

#  ifdef PREFIX
static const char *static_path = PREFIX "/share";
#  else
static const char *static_path = NULL;
#  endif

ANCHOR_SystemPathsUnix::ANCHOR_SystemPathsUnix()
{}

ANCHOR_SystemPathsUnix::~ANCHOR_SystemPathsUnix()
{}

const AnchorU8 *ANCHOR_SystemPathsUnix::getSystemDir(int, const char *versionstr) const
{
  /* no prefix assumes a portable build which only uses bundled scripts */
  if (static_path)
  {
    static string system_path = string(static_path) + "/covah/" + versionstr;
    return (AnchorU8 *)system_path.c_str();
  }

  return NULL;
}

const AnchorU8 *ANCHOR_SystemPathsUnix::getUserDir(int version, const char *versionstr) const
{
  static string user_path = "";
  static int last_version = 0;

  if (version < 264)
  {
    if (user_path.empty() || last_version != version)
    {
      const char *home = getenv("HOME");

      last_version = version;

      if (home)
      {
        user_path = string(home) + "/.covah/" + versionstr;
      }
      else
      {
        return NULL;
      }
    }
    return (AnchorU8 *)user_path.c_str();
  }
  else
  {
    if (user_path.empty() || last_version != version)
    {
      const char *home = getenv("XDG_CONFIG_HOME");

      last_version = version;

      if (home)
      {
        user_path = string(home) + "/covah/" + versionstr;
      }
      else
      {
        home = getenv("HOME");

        if (home == NULL)
          home = getpwuid(getuid())->pw_dir;

        user_path = string(home) + "/.config/covah/" + versionstr;
      }
    }

    return (const AnchorU8 *)user_path.c_str();
  }
}

const AnchorU8 *ANCHOR_SystemPathsUnix::getUserSpecialDir(eAnchorUserSpecialDirTypes type) const
{
  const char *type_str;

  switch (type)
  {
    case ANCHOR_UserSpecialDirDesktop:
      type_str = "DESKTOP";
      break;
    case ANCHOR_UserSpecialDirDocuments:
      type_str = "DOCUMENTS";
      break;
    case ANCHOR_UserSpecialDirDownloads:
      type_str = "DOWNLOAD";
      break;
    case ANCHOR_UserSpecialDirMusic:
      type_str = "MUSIC";
      break;
    case ANCHOR_UserSpecialDirPictures:
      type_str = "PICTURES";
      break;
    case ANCHOR_UserSpecialDirVideos:
      type_str = "VIDEOS";
      break;
    default:
      TF_CODING_ERROR(
        "ANCHOR_SystemPathsUnix::getUserSpecialDir(): Invalid enum value for type parameter\n");
      return NULL;
  }

  static string path = "";
  /* Pipe stderr to /dev/null to avoid error prints. We will fail gracefully still. */
  string command = string("xdg-user-dir ") + type_str + " 2> /dev/null";

  FILE *fstream = popen(command.c_str(), "r");
  if (fstream == NULL)
  {
    return NULL;
  }
  std::stringstream path_stream;
  while (!feof(fstream))
  {
    char c = fgetc(fstream);
    /* xdg-user-dir ends the path with '\n'. */
    if (c == '\n')
    {
      break;
    }
    path_stream << c;
  }
  if (pclose(fstream) == -1)
  {
    perror("ANCHOR_SystemPathsUnix::getUserSpecialDir failed at pclose()");
    return NULL;
  }

  path = path_stream.str();
  return path[0] ? (const AnchorU8 *)path.c_str() : NULL;
}

const AnchorU8 *ANCHOR_SystemPathsUnix::getBinaryDir() const
{
  return NULL;
}

void ANCHOR_SystemPathsUnix::addToSystemRecentFiles(const char * /*filename*/) const
{
  /* TODO: implement for X11 */
}
#endif /* __linux__ */

ANCHOR_ISystemPaths *ANCHOR_ISystemPaths::m_systemPaths = NULL;

eAnchorStatus ANCHOR_ISystemPaths::create()
{
  eAnchorStatus success;
  if (!m_systemPaths)
  {
#ifdef WIN32
    m_systemPaths = new ANCHOR_SystemPathsWin32();
#else
#  ifdef __APPLE__
    m_systemPaths = new ANCHOR_SystemPathsCocoa();
#  else
    m_systemPaths = new ANCHOR_SystemPathsUnix();
#  endif
#endif
    success = m_systemPaths != NULL ? ANCHOR_SUCCESS : ANCHOR_ERROR;
  }
  else
  {
    success = ANCHOR_ERROR;
  }
  return success;
}

eAnchorStatus ANCHOR_ISystemPaths::dispose()
{
  eAnchorStatus success = ANCHOR_SUCCESS;
  if (m_systemPaths)
  {
    delete m_systemPaths;
    m_systemPaths = NULL;
  }
  else
  {
    success = ANCHOR_ERROR;
  }
  return success;
}

ANCHOR_ISystemPaths *ANCHOR_ISystemPaths::get()
{
  if (!m_systemPaths)
  {
    create();
  }
  return m_systemPaths;
}

eAnchorStatus ANCHOR_CreateSystemPaths(void)
{
  return ANCHOR_ISystemPaths::create();
}