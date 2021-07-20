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
 * ⚓︎ Anchor.
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

AnchorSystemPathsUnix::AnchorSystemPathsUnix()
{}

AnchorSystemPathsUnix::~AnchorSystemPathsUnix()
{}

const AnchorU8 *AnchorSystemPathsUnix::getSystemDir(int, const char *versionstr) const
{
  /* no prefix assumes a portable build which only uses bundled scripts */
  if (static_path)
  {
    static string system_path = string(static_path) + "/kraken/" + versionstr;
    return (AnchorU8 *)system_path.c_str();
  }

  return NULL;
}

const AnchorU8 *AnchorSystemPathsUnix::getUserDir(int version, const char *versionstr) const
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
        user_path = string(home) + "/.kraken/" + versionstr;
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
        user_path = string(home) + "/kraken/" + versionstr;
      }
      else
      {
        home = getenv("HOME");

        if (home == NULL)
          home = getpwuid(getuid())->pw_dir;

        user_path = string(home) + "/.config/kraken/" + versionstr;
      }
    }

    return (const AnchorU8 *)user_path.c_str();
  }
}

const AnchorU8 *AnchorSystemPathsUnix::getUserSpecialDir(eAnchorUserSpecialDirTypes type) const
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
        "AnchorSystemPathsUnix::getUserSpecialDir(): Invalid enum value for type parameter\n");
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
    perror("AnchorSystemPathsUnix::getUserSpecialDir failed at pclose()");
    return NULL;
  }

  path = path_stream.str();
  return path[0] ? (const AnchorU8 *)path.c_str() : NULL;
}

const AnchorU8 *AnchorSystemPathsUnix::getBinaryDir() const
{
  return NULL;
}

void AnchorSystemPathsUnix::addToSystemRecentFiles(const char * /*filename*/) const
{
  /* TODO: implement for X11 */
}
#elif defined(_WIN32) /* __linux__ */

#  ifndef _WIN32_IE
#    define _WIN32_IE 0x0501
#  endif
#  include "utfconv.h"
#  include <shlobj.h>

AnchorSystemPathsWin32::AnchorSystemPathsWin32()
{
}

AnchorSystemPathsWin32::~AnchorSystemPathsWin32()
{
}

const AnchorU8 *AnchorSystemPathsWin32::getSystemDir(int, const char *versionstr) const
{
  /* 1 utf-16 might translate into 3 utf-8. 2 utf-16 translates into 4 utf-8. */
  static char knownpath[MAX_PATH * 3 + 128] = {0};
  PWSTR knownpath_16 = NULL;

  HRESULT hResult = SHGetKnownFolderPath(
    FOLDERID_ProgramData, KF_FLAG_DEFAULT, NULL, &knownpath_16);

  if (hResult == S_OK)
  {
    conv_utf_16_to_8(knownpath_16, knownpath, MAX_PATH * 3);
    CoTaskMemFree(knownpath_16);
    strcat(knownpath, "\\Wabi Animation\\Kraken\\");
    strcat(knownpath, versionstr);
    return (AnchorU8 *)knownpath;
  }

  return NULL;
}

const AnchorU8 *AnchorSystemPathsWin32::getUserDir(int, const char *versionstr) const
{
  static char knownpath[MAX_PATH * 3 + 128] = {0};
  PWSTR knownpath_16 = NULL;

  HRESULT hResult = SHGetKnownFolderPath(
    FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, NULL, &knownpath_16);

  if (hResult == S_OK)
  {
    conv_utf_16_to_8(knownpath_16, knownpath, MAX_PATH * 3);
    CoTaskMemFree(knownpath_16);
    strcat(knownpath, "\\Wabi Animation\\Kraken\\");
    strcat(knownpath, versionstr);
    return (AnchorU8 *)knownpath;
  }

  return NULL;
}

const AnchorU8 *AnchorSystemPathsWin32::getUserSpecialDir(eAnchorUserSpecialDirTypes type) const
{
  GUID folderid;

  switch (type)
  {
    case ANCHOR_UserSpecialDirDesktop:
      folderid = FOLDERID_Desktop;
      break;
    case ANCHOR_UserSpecialDirDocuments:
      folderid = FOLDERID_Documents;
      break;
    case ANCHOR_UserSpecialDirDownloads:
      folderid = FOLDERID_Downloads;
      break;
    case ANCHOR_UserSpecialDirMusic:
      folderid = FOLDERID_Music;
      break;
    case ANCHOR_UserSpecialDirPictures:
      folderid = FOLDERID_Pictures;
      break;
    case ANCHOR_UserSpecialDirVideos:
      folderid = FOLDERID_Videos;
      break;
    default:
      TF_MSG_ERROR("Anchor -- Invalid enum value for type parameter");
      return NULL;
  }

  static char knownpath[MAX_PATH * 3] = {0};
  PWSTR knownpath_16 = NULL;
  HRESULT hResult = SHGetKnownFolderPath(folderid, KF_FLAG_DEFAULT, NULL, &knownpath_16);

  if (hResult == S_OK)
  {
    conv_utf_16_to_8(knownpath_16, knownpath, MAX_PATH * 3);
    CoTaskMemFree(knownpath_16);
    return (AnchorU8 *)knownpath;
  }

  CoTaskMemFree(knownpath_16);
  return NULL;
}

const AnchorU8 *AnchorSystemPathsWin32::getBinaryDir() const
{
  static char fullname[MAX_PATH * 3] = {0};
  wchar_t fullname_16[MAX_PATH * 3];

  if (GetModuleFileNameW(0, fullname_16, MAX_PATH))
  {
    conv_utf_16_to_8(fullname_16, fullname, MAX_PATH * 3);
    return (AnchorU8 *)fullname;
  }

  return NULL;
}

void AnchorSystemPathsWin32::addToSystemRecentFiles(const char *filename) const
{
  /* SHARD_PATH resolves to SHARD_PATHA for non-UNICODE build */
  UTF16_ENCODE(filename);
  SHAddToRecentDocs(SHARD_PATHW, filename_16);
  UTF16_UN_ENCODE(filename);
}

#endif /* _WIN32 */


AnchorISystemPaths *AnchorISystemPaths::m_systemPaths = NULL;

eAnchorStatus AnchorISystemPaths::create()
{
  eAnchorStatus success;
  if (!m_systemPaths)
  {
#ifdef WIN32
    m_systemPaths = new AnchorSystemPathsWin32();
#else
#  ifdef __APPLE__
    m_systemPaths = new AnchorSystemPathsCocoa();
#  else
    m_systemPaths = new AnchorSystemPathsUnix();
#  endif
#endif
    success = m_systemPaths != NULL ? ANCHOR_SUCCESS : ANCHOR_FAILURE;
  }
  else
  {
    success = ANCHOR_FAILURE;
  }
  return success;
}

eAnchorStatus AnchorISystemPaths::dispose()
{
  eAnchorStatus success = ANCHOR_SUCCESS;
  if (m_systemPaths)
  {
    delete m_systemPaths;
    m_systemPaths = NULL;
  }
  else
  {
    success = ANCHOR_FAILURE;
  }
  return success;
}

AnchorISystemPaths *AnchorISystemPaths::get()
{
  if (!m_systemPaths)
  {
    create();
  }
  return m_systemPaths;
}

eAnchorStatus ANCHOR_CreateSystemPaths(void)
{
  return AnchorISystemPaths::create();
}

eAnchorStatus ANCHOR_DisposeSystemPaths(void)
{
  return AnchorISystemPaths::dispose();
}

const AnchorU8 *ANCHOR_getSystemDir(int version, const char *versionstr)
{
  AnchorISystemPaths *systemPaths = AnchorISystemPaths::get();
  return systemPaths ? systemPaths->getSystemDir(version, versionstr) : NULL;
}

const AnchorU8 *ANCHOR_getUserDir(int version, const char *versionstr)
{
  AnchorISystemPaths *systemPaths = AnchorISystemPaths::get();
  return systemPaths ? systemPaths->getUserDir(version, versionstr) : NULL; /* shouldn't be NULL */
}

const AnchorU8 *ANCHOR_getUserSpecialDir(eAnchorUserSpecialDirTypes type)
{
  AnchorISystemPaths *systemPaths = AnchorISystemPaths::get();
  return systemPaths ? systemPaths->getUserSpecialDir(type) : NULL; /* shouldn't be NULL */
}

const AnchorU8 *ANCHOR_getBinaryDir()
{
  AnchorISystemPaths *systemPaths = AnchorISystemPaths::get();
  return systemPaths ? systemPaths->getBinaryDir() : NULL; /* shouldn't be NULL */
}

void ANCHOR_addToSystemRecentFiles(const char *filename)
{
  AnchorISystemPaths *systemPaths = AnchorISystemPaths::get();
  if (systemPaths)
  {
    systemPaths->addToSystemRecentFiles(filename);
  }
}