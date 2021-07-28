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
 * KRAKEN Library.
 * Gadget Vault. */

#ifdef WIN32

#  include <winrt/base.h>
#  include <winrt/Windows.Foundation.h>
#  include <winrt/Windows.Storage.h>

using namespace winrt;
using namespace winrt::Windows;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;

#include <wabi/base/tf/diagnostic.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/base/arch/systemInfo.h>

#  define WIN32_SKIP_HKEY_PROTECTION  // need to use HKEY
#  include "KLI_path_utils.h"
#  include "KLI_string_utils.h"
#  include "KLI_utildefines.h"
#  include "KLI_winstuff.h"
#  include "utfconv.h"

#  define PATH_SUFFIX "\\*"
#  define PATH_SUFFIX_LEN 2

#  include <string.h>
#  include <conio.h>
#  include <stdio.h>
#  include <stdlib.h>

using namespace std;

WABI_NAMESPACE_BEGIN

/* keep local to this file */
struct __dirstream
{
  HANDLE handle;
  WIN32_FIND_DATAW data;
  char path[MAX_PATH + PATH_SUFFIX_LEN];
  long dd_loc;
  long dd_size;
  char dd_buf[4096];
  void *dd_direct;

  struct dirent direntry;
};

/**
 * \note MinGW (FREE_WINDOWS) has #opendir() and #_wopendir(), and only the
 * latter accepts a path name of #wchar_t type. Rather than messing up with
 * extra #ifdef's here and there, Blender's own implementations of #opendir()
 * and related functions are used to properly support paths with non-ASCII
 * characters. (kjym3) */

DIR *opendir(const char *path)
{
  wchar_t *path_16 = alloc_utf16_from_8(path, 0);
  int path_len;
  DIR *newd = NULL;

  if ((GetFileAttributesW(path_16) & FILE_ATTRIBUTE_DIRECTORY) &&
      ((path_len = strlen(path)) < (sizeof(newd->path) - PATH_SUFFIX_LEN)))
  {
    newd = (DIR *)malloc(sizeof(DIR));
    newd->handle = INVALID_HANDLE_VALUE;
    memcpy(newd->path, path, path_len);
    memcpy(newd->path + path_len, PATH_SUFFIX, PATH_SUFFIX_LEN + 1);

    newd->direntry.d_ino = 0;
    newd->direntry.d_off = 0;
    newd->direntry.d_reclen = 0;
    newd->direntry.d_name = NULL;
  }

  free(path_16);
  return newd;
}

static char *KLI_alloc_utf_8_from_16(wchar_t *in16, size_t add)
{
  size_t bsize = count_utf_8_from_16(in16);
  char *out8 = NULL;
  if (!bsize)
  {
    return NULL;
  }
  out8 = (char *)malloc(sizeof(char) * (bsize + add));
  conv_utf_16_to_8(in16, out8, bsize);
  return out8;
}

static wchar_t *UNUSED_FUNCTION(KLI_alloc_utf16_from_8)(char *in8, size_t add)
{
  size_t bsize = count_utf_16_from_8(in8);
  wchar_t *out16 = NULL;
  if (!bsize)
  {
    return NULL;
  }
  out16 = (wchar_t *)malloc(sizeof(wchar_t) * (bsize + add));
  conv_utf_8_to_16(in8, out16, bsize);
  return out16;
}

struct dirent *readdir(DIR *dp)
{
  if (dp->direntry.d_name)
  {
    free(dp->direntry.d_name);
    dp->direntry.d_name = NULL;
  }

  if (dp->handle == INVALID_HANDLE_VALUE)
  {
    wchar_t *path_16 = alloc_utf16_from_8(dp->path, 0);
    dp->handle = FindFirstFileW(path_16, &(dp->data));
    free(path_16);
    if (dp->handle == INVALID_HANDLE_VALUE)
    {
      return NULL;
    }

    dp->direntry.d_name = KLI_alloc_utf_8_from_16(dp->data.cFileName, 0);

    return &dp->direntry;
  } else if (FindNextFileW(dp->handle, &(dp->data)))
  {
    dp->direntry.d_name = KLI_alloc_utf_8_from_16(dp->data.cFileName, 0);

    return &dp->direntry;
  } else
  {
    return NULL;
  }
}

int closedir(DIR *dp)
{
  if (dp->direntry.d_name)
  {
    free(dp->direntry.d_name);
  }
  if (dp->handle != INVALID_HANDLE_VALUE)
  {
    FindClose(dp->handle);
  }

  free(dp);

  return 0;
}

/* FILE_MAXDIR + FILE_MAXFILE */

int KLI_windows_get_executable_dir(char *str)
{
  strcpy(str, CHARALL(TfGetPathName(ArchGetExecutablePath())));

  return 1;
}

static void register_pixar_extension_failed(HKEY root, const bool background)
{
  printf("failed\n");
  if (root)
  {
    RegCloseKey(root);
  }
  if (!background)
  {
    TF_MSG_ERROR("Could not register file extension.");
    // MessageBox(0, "Could not register file extension.", "Kraken error", MB_OK | MB_ICONERROR);
  }
}

bool KLI_windows_register_pixar_extension(const bool background)
{
  LONG lresult;
  HKEY hkey = 0;
  HKEY root = 0;
  BOOL usr_mode = false;
  DWORD dwd = 0;
  char buffer[256];

  char KrPath[MAX_PATH];
  char InstallDir[FILE_MAXDIR];
  char SysDir[FILE_MAXDIR];
  const char *ThumbHandlerDLL;
  char RegCmd[MAX_PATH * 2];
  char MBox[256];
  char *kraken_app;
#  ifndef _WIN64
  BOOL IsWOW64;
#  endif

  printf("Registering file extension...");
  // GetModuleFileName(0, KrPath, MAX_PATH);

  /* Replace the actual app name with the wrapper. */
  // kraken_app = strstr(KrPath, "kraken.exe");
  // if (kraken_app != NULL)
  // {
  //   strcpy(kraken_app, "kraken-launcher.exe");
  // }

  // /* root is HKLM by default */
  // lresult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Classes", 0, KEY_ALL_ACCESS, &root);
  // if (lresult != ERROR_SUCCESS)
  // {
  //   /* try HKCU on failure */
  //   usr_mode = true;
  //   lresult = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Classes", 0, KEY_ALL_ACCESS, &root);
  //   if (lresult != ERROR_SUCCESS)
  //   {
  //     register_pixar_extension_failed(0, background);
  //     return false;
  //   }
  // }

  // lresult =
  //   RegCreateKeyEx(root, "pixarfile", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwd);
  // if (lresult == ERROR_SUCCESS)
  // {
  //   strcpy(buffer, "Kraken File");
  //   lresult = RegSetValueEx(hkey, NULL, 0, REG_SZ, (BYTE *)buffer, strlen(buffer) + 1);
  //   RegCloseKey(hkey);
  // }
  // if (lresult != ERROR_SUCCESS)
  // {
  //   register_pixar_extension_failed(root, background);
  //   return false;
  // }

  // lresult = RegCreateKeyEx(root,
  //                          "pixarfile\\shell\\open\\command",
  //                          0,
  //                          NULL,
  //                          REG_OPTION_NON_VOLATILE,
  //                          KEY_ALL_ACCESS,
  //                          NULL,
  //                          &hkey,
  //                          &dwd);
  // if (lresult == ERROR_SUCCESS)
  // {
  //   sprintf(buffer, "\"%s\" \"%%1\"", KrPath);
  //   lresult = RegSetValueEx(hkey, NULL, 0, REG_SZ, (BYTE *)buffer, strlen(buffer) + 1);
  //   RegCloseKey(hkey);
  // }
  // if (lresult != ERROR_SUCCESS)
  // {
  //   register_pixar_extension_failed(root, background);
  //   return false;
  // }

  // lresult = RegCreateKeyEx(root,
  //                          "pixarfile\\DefaultIcon",
  //                          0,
  //                          NULL,
  //                          REG_OPTION_NON_VOLATILE,
  //                          KEY_ALL_ACCESS,
  //                          NULL,
  //                          &hkey,
  //                          &dwd);
  // if (lresult == ERROR_SUCCESS)
  // {
  //   sprintf(buffer, "\"%s\", 1", KrPath);
  //   lresult = RegSetValueEx(hkey, NULL, 0, REG_SZ, (BYTE *)buffer, strlen(buffer) + 1);
  //   RegCloseKey(hkey);
  // }
  // if (lresult != ERROR_SUCCESS)
  // {
  //   register_pixar_extension_failed(root, background);
  //   return false;
  // }

  // lresult =
  //   RegCreateKeyEx(root, ".usd", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwd);
  // if (lresult == ERROR_SUCCESS)
  // {
  //   strcpy(buffer, "pixarfile");
  //   lresult = RegSetValueEx(hkey, NULL, 0, REG_SZ, (BYTE *)buffer, strlen(buffer) + 1);
  //   RegCloseKey(hkey);
  // }
  // if (lresult != ERROR_SUCCESS)
  // {
  //   register_pixar_extension_failed(root, background);
  //   return false;
  // }

  // KLI_windows_get_executable_dir(InstallDir);
  // GetSystemDirectory(SysDir, FILE_MAXDIR);
  // ThumbHandlerDLL = "KrakenThumb.dll";
  // snprintf(RegCmd, MAX_PATH * 2, "%s\\regsvr32 /s \"%s\\%s\"", SysDir, InstallDir, ThumbHandlerDLL);
  // system(RegCmd);

  // RegCloseKey(root);
  // printf("success (%s)\n", usr_mode ? "user" : "system");
  // if (!background)
  // {
  //   sprintf(MBox,
  //           "File extension registered for %s.",
  //           usr_mode ? "the current user. To register for all users, run as an administrator" : "all users");
  //   // MessageBox(0, MBox, "Kraken", MB_OK | MB_ICONINFORMATION);
  // }
  return true;
}

void KLI_windows_get_default_root_dir(char *root)
{
  // char str[MAX_PATH + 1];

  // /* the default drive to resolve a directory without a specified drive
  //  * should be the Windows installation drive, since this was what the OS
  //  * assumes. */
  // if (GetWindowsDirectory(str, MAX_PATH + 1))
  // {
  //   root[0] = str[0];
  //   root[1] = ':';
  //   root[2] = '\\';
  //   root[3] = '\0';
  // } else
  // {
  //   /* if GetWindowsDirectory fails, something has probably gone wrong,
  //    * we are trying the kraken install dir though */
  //   if (GetModuleFileName(NULL, str, MAX_PATH + 1))
  //   {
  //     printf(
  //       "Error! Could not get the Windows Directory - "
  //       "Defaulting to Kraken installation Dir!\n");
  //     root[0] = str[0];
  //     root[1] = ':';
  //     root[2] = '\\';
  //     root[3] = '\0';
  //   } else
  //   {
  //     DWORD tmp;
  //     int i;
  //     int rc = 0;
  //     /* now something has gone really wrong - still trying our best guess */
  //     printf(
  //       "Error! Could not get the Windows Directory - "
  //       "Defaulting to first valid drive! Path might be invalid!\n");
  //     tmp = GetLogicalDrives();
  //     for (i = 2; i < 26; i++)
  //     {
  //       if ((tmp >> i) & 1)
  //       {
  //         root[0] = 'a' + i;
  //         root[1] = ':';
  //         root[2] = '\\';
  //         root[3] = '\0';
  //         if (GetFileAttributes(root) != 0xFFFFFFFF)
  //         {
  //           rc = i;
  //           break;
  //         }
  //       }
  //     }
  //     if (0 == rc)
  //     {
  //       printf("ERROR in 'KLI_windows_get_default_root_dir': can't find a valid drive!\n");
  //       root[0] = 'C';
  //       root[1] = ':';
  //       root[2] = '\\';
  //       root[3] = '\0';
  //     }
  //   }
  // }
}

WABI_NAMESPACE_END

#else

/* intentionally empty for UNIX */

#endif
