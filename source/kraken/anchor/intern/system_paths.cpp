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

#include "KLI_utildefines.h"

#include "ANCHOR_system_paths.h"

#include <wabi/base/arch/defines.h>
#include <wabi/base/arch/systemInfo.h>

#if defined(ARCH_OS_WINDOWS)
#  include <winrt/base.h>
#  include <winrt/Windows.ApplicationModel.h>
#  include <winrt/Windows.Foundation.h>
#  include <winrt/Windows.Storage.h>

namespace MICROSOFT = winrt;

using namespace MICROSOFT;
using namespace MICROSOFT::Windows;
using namespace MICROSOFT::Windows::ApplicationModel;
using namespace MICROSOFT::Windows::Foundation;
using namespace MICROSOFT::Windows::Storage;
#endif /* ARCH_OS_WINDOWS */

WABI_NAMESPACE_USING

#include <stdio.h>

#if defined(ARCH_OS_LINUX)
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
      } else
      {
        return NULL;
      }
    }
    return (AnchorU8 *)user_path.c_str();
  } else
  {
    if (user_path.empty() || last_version != version)
    {
      const char *home = getenv("XDG_CONFIG_HOME");

      last_version = version;

      if (home)
      {
        user_path = string(home) + "/kraken/" + versionstr;
      } else
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
      TF_CODING_ERROR("AnchorSystemPathsUnix::getUserSpecialDir(): Invalid enum value for type parameter\n");
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
#elif defined(ARCH_OS_WINDOWS) /* ARCH_OS_LINUX */

#  ifndef _WIN32_IE
#    define _WIN32_IE 0x0501
#  endif
#  include "utfconv.h"
#  include <shlobj.h>

AnchorSystemPathsWin32::AnchorSystemPathsWin32()
{}

AnchorSystemPathsWin32::~AnchorSystemPathsWin32()
{}

const AnchorU8 *AnchorSystemPathsWin32::getSystemDir(int, const char *versionstr) const
{
  std::string sysDir = MICROSOFT::to_string(Package::Current().InstalledLocation().Path());

  if (!sysDir.empty())
  {
    return (AnchorU8 *)CHARSTR(STRCAT(sysDir, versionstr));
  }

  return NULL;
}

const AnchorU8 *AnchorSystemPathsWin32::getUserDir(int, const char *versionstr) const
{
  fs::path userDir = STRCAT(fs::temp_directory_path().string(), STRCAT("../../", versionstr));

  if (!userDir.empty())
  {
    return (AnchorU8 *)CHARSTR(userDir.string());
  }

  return NULL;
}

const AnchorU8 *AnchorSystemPathsWin32::getUserSpecialDir(eAnchorUserSpecialDirTypes type) const
{
  MICROSOFT::hstring folderid;

  switch (type)
  {
    case ANCHOR_UserSpecialDirDesktop:
      folderid = AppDataPaths::GetDefault().Desktop();
      break;
    case ANCHOR_UserSpecialDirDocuments:
      folderid = Storage::KnownFolders::DocumentsLibrary().Path();
      break;
    case ANCHOR_UserSpecialDirDownloads:
      folderid = Storage::KnownFolders::Objects3D().Path();
      break;
    case ANCHOR_UserSpecialDirMusic:
      folderid = Storage::KnownFolders::MusicLibrary().Path();
      break;
    case ANCHOR_UserSpecialDirPictures:
      folderid = Storage::KnownFolders::PicturesLibrary().Path();
      break;
    case ANCHOR_UserSpecialDirVideos:
      folderid = Storage::KnownFolders::VideosLibrary().Path();
      break;
    default:
      TF_MSG_ERROR("Anchor -- Invalid enum value for type parameter");
      return NULL;
  }

  if (!folderid.empty())
  {
    return (AnchorU8 *)CHARSTR(folderid);
  }

  return NULL;
}

const AnchorU8 *AnchorSystemPathsWin32::getBinaryDir() const
{
  return (AnchorU8 *)CHARSTR(TfGetPathName(ArchGetExecutablePath()));
}

void AnchorSystemPathsWin32::addToSystemRecentFiles(const char *filename) const
{
  MICROSOFT::Windows::Storage::StorageLibrary documents{
    Storage::StorageLibrary::GetLibraryAsync(Storage::KnownLibraryId::Documents).GetResults()};

  MICROSOFT::Windows::Storage::StorageFolder recentFiles{
    documents.RequestAddFolderAsync().GetResults()};

  MICROSOFT::Windows::Storage::StorageFile fileAdded{
    recentFiles.CreateFileAsync((LPWSTR)filename).GetResults()};

  if (fileAdded.Path().empty())
  {
    TF_MSG_ERROR("ANCHOR - Error adding file to System Recent files.");
  }
}

#endif /* ARCH_OS_WINDOWS */


AnchorISystemPaths *AnchorISystemPaths::m_systemPaths = NULL;

eAnchorStatus AnchorISystemPaths::create()
{
  eAnchorStatus success;
  if (!m_systemPaths)
  {
#if defined(ARCH_OS_WINDOWS)
    m_systemPaths = new AnchorSystemPathsWin32();
#elif defined(ARCH_OS_DARWIN)
    m_systemPaths = new AnchorSystemPathsCocoa();
#else /* defined(ARCH_OS_LINUX) */
    m_systemPaths = new AnchorSystemPathsUnix();
#endif
    success = m_systemPaths != NULL ? ANCHOR_SUCCESS : ANCHOR_FAILURE;
  } else
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
  } else
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