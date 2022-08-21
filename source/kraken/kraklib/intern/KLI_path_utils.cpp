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

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#include "KLI_api.h"
#include "KLI_assert.h"
#include "KLI_fileops.h"
#include "KLI_string_utils.h"
#include "KLI_path_utils.h"

#include <cstring>
#include <stdarg.h>
#include <string>

#ifdef WIN32
#  include "utfconv.h"
#  include <io.h>
#  ifdef _WIN32_IE
#    undef _WIN32_IE
#  endif
#  define _WIN32_IE 0x0501
#  include <shlobj.h>
#  include <windows.h>

#  include "KLI_winstuff.h"

#else /* non windows */
#  ifdef WITH_BINRELOC
#    include "binreloc.h"
#  endif
/* #mkdtemp on OSX (and probably all *BSD?), not worth making specific check for this OS. */
#  include <unistd.h>
#endif /* WIN32 */

#include <wabi/base/arch/hints.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/base/tf/iterator.h>
#include <wabi/base/tf/pathUtils.h>

WABI_NAMESPACE_BEGIN

#ifdef WIN32

/**
 * Return true if the path is absolute ie starts with a drive specifier
 * (eg A:\) or is a UNC path.
 */
static bool KLI_path_is_abs(const char *name);

#endif /* WIN32 */

/**
 * Get an env var, result has to be used immediately.
 *
 * On windows #getenv gets its variables from a static copy of the environment variables taken at
 * process start-up, causing it to not pick up on environment variables created during runtime.
 * This function uses an alternative method to get environment variables that does pick up on
 * runtime environment variables. The result will be UTF-8 encoded. */
const char *KLI_getenv(const char *env)
{
#ifdef _MSC_VER
  const char *result = NULL;
  /* 32767 is the maximum size of the environment variable on windows,
   * reserve one more character for the zero terminator. */
  static wchar_t buffer[32768];
  wchar_t *env_16 = alloc_utf16_from_8(env, 0);
  if (env_16) {
    if (GetEnvironmentVariableW(env_16, buffer, TfArraySize(buffer))) {
      char *res_utf8 = alloc_utf_8_from_16(buffer, 0);
      /* Make sure the result is valid, and will fit into our temporary storage buffer. */
      if (res_utf8) {
        if (strlen(res_utf8) + 1 < sizeof(buffer)) {
          /* We are re-using the utf16 buffer here, since allocating a second static buffer to
           * contain the UTF-8 version to return would be wasteful. */
          memcpy(buffer, res_utf8, strlen(res_utf8) + 1);
          result = (const char *)buffer;
        }
        free(res_utf8);
      }
    }
  }
  return result;
#else
  return getenv(env);
#endif
}

#if defined(WIN32)

/**
 * Return true if the path is absolute ie starts with a drive specifier
 * (eg A:\) or is a UNC path. */
static bool KLI_path_is_abs(const char *name)
{
  return (name[1] == ':' && ((name[2] == '\\') || (name[2] == '/'))) || KLI_path_is_unc(name);
}

#endif /* WIN32 */

static int KLI_path_unc_prefix_len(const char *path);

/**
 * Simple appending of filename to dir, does not check for valid path!
 * Puts result into `dst`, which may be same area as `dir`. */
void KLI_join_dirfile(char *__restrict dst,
                      const size_t maxlen,
                      const char *__restrict dir,
                      const char *__restrict file)
{
#ifdef DEBUG_STRSIZE
  memset(dst, 0xff, sizeof(*dst) * maxlen);
#endif
  size_t dirlen = KLI_strnlen(dir, maxlen);

  /* Arguments can't match. */
  KLI_assert(!ELEM(dst, dir, file));

  /* Files starting with a separator cause a double-slash which could later be interpreted
   * as a relative path where: `dir == "/"` and `file == "/file"` would result in "//file". */
  KLI_assert(file[0] != SEP);

  if (dirlen == maxlen) {
    memcpy(dst, dir, dirlen);
    dst[dirlen - 1] = '\0';
    return; /* dir fills the path */
  }

  memcpy(dst, dir, dirlen + 1);

  if (dirlen + 1 >= maxlen) {
    return; /* fills the path */
  }

  /* inline KLI_path_slash_ensure */
  if ((dirlen > 0) && !ELEM(dst[dirlen - 1], SEP, ALTSEP)) {
    dst[dirlen++] = SEP;
    dst[dirlen] = '\0';
  }

  if (dirlen >= maxlen) {
    return; /* fills the path */
  }

  KLI_strncpy(dst + dirlen, file, maxlen - dirlen);
}

/**
 * Joins infinite strings into @param dst,
 * ensuring only a single path separator between each. */
size_t KLI_path_join(char *__restrict dst, const size_t dst_len, const char *path, ...)
{
#ifdef DEBUG_STRSIZE
  memset(dst, 0xff, sizeof(*dst) * dst_len);
#endif
  if (ARCH_UNLIKELY(dst_len == 0)) {
    return 0;
  }
  const size_t dst_last = dst_len - 1;
  size_t ofs = KLI_strncpy_rlen(dst, path, dst_len);

  if (ofs == dst_last) {
    return ofs;
  }

  /* remove trailing slashes, unless there are _only_ trailing slashes
   * (allow "//" as the first argument). */
  bool has_trailing_slash = false;
  if (ofs != 0) {
    size_t len = ofs;
    while ((len != 0) && ELEM(path[len - 1], SEP, ALTSEP)) {
      len -= 1;
    }
    if (len != 0) {
      ofs = len;
    }
    has_trailing_slash = (path[len] != '\0');
  }

  va_list args;
  va_start(args, path);
  while ((path = (const char *)va_arg(args, const char *))) {
    has_trailing_slash = false;
    const char *path_init = path;
    while (ELEM(path[0], SEP, ALTSEP)) {
      path++;
    }
    size_t len = strlen(path);
    if (len != 0) {
      while ((len != 0) && ELEM(path[len - 1], SEP, ALTSEP)) {
        len -= 1;
      }

      if (len != 0) {
        /* the very first path may have a slash at the end */
        if (ofs && !ELEM(dst[ofs - 1], SEP, ALTSEP)) {
          dst[ofs++] = SEP;
          if (ofs == dst_last) {
            break;
          }
        }
        has_trailing_slash = (path[len] != '\0');
        if (ofs + len >= dst_last) {
          len = dst_last - ofs;
        }
        memcpy(&dst[ofs], path, len);
        ofs += len;
        if (ofs == dst_last) {
          break;
        }
      }
    } else {
      has_trailing_slash = (path_init != path);
    }
  }
  va_end(args);

  if (has_trailing_slash) {
    if ((ofs != dst_last) && (ofs != 0) && (ELEM(dst[ofs - 1], SEP, ALTSEP) == 0)) {
      dst[ofs++] = SEP;
    }
  }

  KLI_assert(ofs <= dst_last);
  dst[ofs] = '\0';

  return ofs;
}


/**
 * Does path begin with the special "//" prefix that
 * Kraken uses to indicate a path relative to the
 * .usd file. */
bool KLI_path_is_rel(const char *path)
{
  return path[0] == '/' && path[1] == '/';
}

/* return true if the path is a UNC share */
bool KLI_path_is_unc(const char *name)
{
  return name[0] == '\\' && name[1] == '\\';
}

/**
 * Returns the length of the identifying prefix
 * of a UNC path which can start with '\\' (short version)
 * or '\\?\' (long version)
 * If the path is not a UNC path, return 0 */
static int KLI_path_unc_prefix_len(const char *path)
{
  if (KLI_path_is_unc(path)) {
    if ((path[2] == '?') && (path[3] == '\\')) {
      /* we assume long UNC path like \\?\server\share\folder etc... */
      return 4;
    }

    return 2;
  }

  return 0;
}

static bool path_extension_check_ex(const char *str,
                                    const size_t str_len,
                                    const char *ext,
                                    const size_t ext_len)
{
  KLI_assert(strlen(str) == str_len);
  KLI_assert(strlen(ext) == ext_len);

  return (((str_len == 0 || ext_len == 0 || ext_len >= str_len) == 0) &&
          (KLI_strcasecmp(ext, str + str_len - ext_len) == 0));
}

bool KLI_path_extension_check(const char *str, const char *ext)
{
  return path_extension_check_ex(str, strlen(str), ext, strlen(ext));
}

bool KLI_path_extension_check_n(const char *str, ...)
{
  const size_t str_len = strlen(str);

  va_list args;
  const char *ext;
  bool ret = false;

  va_start(args, str);

  while ((ext = (const char *)va_arg(args, void *))) {
    if (path_extension_check_ex(str, str_len, ext, strlen(ext))) {
      ret = true;
      break;
    }
  }

  va_end(args);

  return ret;
}

/* does str end with any of the suffixes in *ext_array. */
bool KLI_path_extension_check_array(const std::string &str, const char **ext_array)
{
  const size_t str_len = str.length();
  int i = 0;

  while (ext_array[i]) {
    if (path_extension_check_ex(str.c_str(), str_len, ext_array[i], strlen(ext_array[i]))) {
      return true;
    }

    i++;
  }
  return false;
}


/**
 * Converts `/foo/bar.txt` to `/foo/` and `bar.txt`
 *
 * - Won't change @a string.
 * - Won't create any directories.
 * - Doesn't use CWD, or deal with relative paths.
 * - Only fill's in @a dir and @a file when they are non NULL. */
void KLI_split_dirfile(const char *string,
                       char *dir,
                       char *file,
                       const size_t dirlen,
                       const size_t filelen)
{
#ifdef DEBUG_STRSIZE
  memset(dir, 0xff, sizeof(*dir) * dirlen);
  memset(file, 0xff, sizeof(*file) * filelen);
#endif
  const char *lslash_str = KLI_path_slash_rfind(string);
  const size_t lslash = lslash_str ? (size_t)(lslash_str - string) + 1 : 0;

  if (dir) {
    if (lslash) {
      /* +1 to include the slash and the last char */
      KLI_strncpy(dir, string, MIN2(dirlen, lslash + 1));
    } else {
      dir[0] = '\0';
    }
  }

  if (file) {
    KLI_strncpy(file, string + lslash, filelen);
  }
}


/**
 * Copies the parent directory part of string into `dir`, max length `dirlen`. */
void KLI_split_dir_part(const char *string, char *dir, const size_t dirlen)
{
  KLI_split_dirfile(string, dir, NULL, dirlen, 0);
}

bool KLI_has_pixar_extension(const std::string &str)
{
  const char *ext_test[5] = {".usd", ".usda", ".usdc", ".usdz", NULL};
  return KLI_path_extension_check_array(str, ext_test);
}

/**
 * Append a filename to a dir, ensuring slash separates.
 */
void KLI_path_append(char *__restrict dst, const size_t maxlen, const char *__restrict file)
{
  size_t dirlen = KLI_strnlen(dst, maxlen);

  /* inline KLI_path_slash_ensure */
  if ((dirlen > 0) && (dst[dirlen - 1] != SEP)) {
    dst[dirlen++] = SEP;
    dst[dirlen] = '\0';
  }

  if (dirlen >= maxlen) {
    return; /* fills the path */
  }

  KLI_strncpy(dst + dirlen, file, maxlen - dirlen);
}

static int KLI_path_unc_prefix_len(const char *path); /* defined below in same file */

/**
 * If path begins with "//", strips that and replaces it with `basepath` directory.
 *
 * @note Also converts drive-letter prefix to something more sensible
 * if this is a non-drive-letter-based system.
 *
 * @param path: The path to convert.
 * @param basepath: The directory to base relative paths with.
 * @return true if the path was relative (started with "//"). */
bool KLI_path_abs(char *path, const char *basepath)
{
  const bool wasrelative = KLI_path_is_rel(path);
  char tmp[FILE_MAX];
  char base[FILE_MAX];
#ifdef WIN32

  /* without this: "" --> "C:\" */
  if (*path == '\0') {
    return wasrelative;
  }

  /* we are checking here if we have an absolute path that is not in the current
   * blend file as a lib main - we are basically checking for the case that a
   * UNIX root '/' is passed.
   */
  if (!wasrelative && !KLI_path_is_abs(path)) {
    char *p = path;
    KLI_windows_get_default_root_dir(tmp);
    /* Get rid of the slashes at the beginning of the path. */
    while ((*p == '\\') || (*p == '/')) {
      p++;
    }
    strcat(tmp, p);
  } else {
    KLI_strncpy(tmp, path, FILE_MAX);
  }
#else
  KLI_strncpy(tmp, path, sizeof(tmp));

  /* Check for loading a MS-Windows path on a POSIX system
   * in this case, there is no use in trying `C:/` since it
   * will never exist on a Unix system.
   *
   * Add a `/` prefix and lowercase the drive-letter, remove the `:`.
   * `C:\foo.JPG` -> `/c/foo.JPG` */

  // if (isalpha(tmp[0]) && (tmp[1] == ':') && ELEM(tmp[2], '\\', '/')) {
  // tmp[1] = tolower(tmp[0]); /* Replace ':' with drive-letter. */
  // tmp[0] = '/';
  /* `\` the slash will be converted later. */
  // }

#endif

  /* push slashes into unix mode - strings entering this part are
   * potentially messed up: having both back- and forward slashes.
   * Here we push into one conform direction, and at the end we
   * push them into the system specific dir. This ensures uniformity
   * of paths and solving some problems (and prevent potential future
   * ones) -jesterKing.
   * For UNC paths the first characters containing the UNC prefix
   * shouldn't be switched as we need to distinguish them from
   * paths relative to the .blend file -elubie */
  KLI_str_replace_char(tmp + KLI_path_unc_prefix_len(tmp), '\\', '/');

  /* Paths starting with // will get the blend file as their base,
   * this isn't standard in any os but is used in blender all over the place */
  if (wasrelative) {
    const char *lslash;
    KLI_strncpy(base, basepath, sizeof(base));

    /* file component is ignored, so don't bother with the trailing slash */
    KLI_path_normalize(NULL, base);
    lslash = KLI_path_slash_rfind(base);
    KLI_str_replace_char(base + KLI_path_unc_prefix_len(base), '\\', '/');

    if (lslash) {
      /* length up to and including last "/" */
      const int baselen = (int)(lslash - base) + 1;
      /* use path for temp storage here, we copy back over it right away */
      KLI_strncpy(path, tmp + 2, FILE_MAX); /* strip "//" */

      memcpy(tmp, base, baselen); /* prefix with base up to last "/" */
      KLI_strncpy(tmp + baselen, path, sizeof(tmp) - baselen); /* append path after "//" */
      KLI_strncpy(path, tmp, FILE_MAX);                        /* return as result */
    } else {
      /* base doesn't seem to be a directory--ignore it and just strip "//" prefix on path */
      KLI_strncpy(path, tmp + 2, FILE_MAX);
    }
  } else {
    /* base ignored */
    KLI_strncpy(path, tmp, FILE_MAX);
  }

#ifdef WIN32
  /* skip first two chars, which in case of
   * absolute path will be drive:/blabla and
   * in case of relpath //blabla/. So relpath
   * // will be retained, rest will be nice and
   * shiny win32 backward slashes :) -jesterKing
   */
  KLI_str_replace_char(path + 2, '/', '\\');
#endif

  /* ensure this is after correcting for path switch */
  KLI_path_normalize(NULL, path);

  return wasrelative;
}

/**
 * Remove redundant characters from @a path and optionally make absolute.
 *
 * @param relabase: The path this is relative to, or ignored when NULL.
 * @param path: Can be any input, and this function converts it to a regular full path.
 * Also removes garbage from directory paths, like `/../` or double slashes etc.
 *
 * @note @a path isn't protected for max string names... */
void KLI_path_normalize(const char *relabase, char *path)
{
  ptrdiff_t a;
  char *start, *eind;
  if (relabase) {
    KLI_path_abs(path, relabase);
  } else {
    if (path[0] == '/' && path[1] == '/') {
      if (path[2] == '\0') {
        return; /* path is "//" - can't clean it */
      }
      path = path + 2; /* leave the initial "//" untouched */
    }
  }

  /* Note
   *   memmove(start, eind, strlen(eind) + 1);
   * is the same as
   *   strcpy(start, eind);
   * except strcpy should not be used because there is overlap,
   * so use memmove's slightly more obscure syntax - Campbell
   */

#ifdef WIN32
  while ((start = strstr(path, "\\..\\"))) {
    eind = start + strlen("\\..\\") - 1;
    a = start - path - 1;
    while (a > 0) {
      if (path[a] == '\\') {
        break;
      }
      a--;
    }
    if (a < 0) {
      break;
    } else {
      memmove(path + a, eind, strlen(eind) + 1);
    }
  }

  while ((start = strstr(path, "\\.\\"))) {
    eind = start + strlen("\\.\\") - 1;
    memmove(start, eind, strlen(eind) + 1);
  }

  /* remove two consecutive backslashes, but skip the UNC prefix,
   * which needs to be preserved */
  while ((start = strstr(path + KLI_path_unc_prefix_len(path), "\\\\"))) {
    eind = start + strlen("\\\\") - 1;
    memmove(start, eind, strlen(eind) + 1);
  }
#else
  while ((start = strstr(path, "/../"))) {
    a = start - path - 1;
    if (a > 0) {
      /* <prefix>/<parent>/../<postfix> => <prefix>/<postfix> */
      eind = start + (4 - 1) /* strlen("/../") - 1 */; /* strip "/.." and keep last "/" */
      while (a > 0 && path[a] != '/') {                /* find start of <parent> */
        a--;
      }
      memmove(path + a, eind, strlen(eind) + 1);
    } else {
      /* support for odd paths: eg /../home/me --> /home/me
       * this is a valid path in blender but we can't handle this the usual way below
       * simply strip this prefix then evaluate the path as usual.
       * pythons os.path.normpath() does this */

      /* Note: previous version of following call used an offset of 3 instead of 4,
       * which meant that the "/../home/me" example actually became "home/me".
       * Using offset of 3 gives behavior consistent with the aforementioned
       * Python routine. */
      memmove(path, path + 3, strlen(path + 3) + 1);
    }
  }

  while ((start = strstr(path, "/./"))) {
    eind = start + (3 - 1) /* strlen("/./") - 1 */;
    memmove(start, eind, strlen(eind) + 1);
  }

  while ((start = strstr(path, "//"))) {
    eind = start + (2 - 1) /* strlen("//") - 1 */;
    memmove(start, eind, strlen(eind) + 1);
  }
#endif
}

/**
 * Replaces path with the path of its parent directory, returning true if
 * it was able to find a parent directory within the path.
 */
bool KLI_path_parent_dir(char *path)
{
  const char parent_dir[] = {'.', '.', SEP, '\0'}; /* "../" or "..\\" */
  char tmp[FILE_MAX + 4];

  KLI_join_dirfile(tmp, sizeof(tmp), path, parent_dir);
  KLI_path_normalize(NULL, tmp); /* does all the work of normalizing the path for us */

  if (!KLI_path_extension_check(tmp, parent_dir)) {
    strcpy(path, tmp); /* We assume pardir is always shorter... */
    return true;
  }

  return false;
}

/**
 * Strips off nonexistent (or non-accessible) sub-directories from the end of `dir`,
 * leaving the path of the lowest-level directory that does exist and we can read.
 */
bool KLI_path_parent_dir_until_exists(char *dir)
{
  bool valid_path = true;

  /* Loop as long as cur path is not a dir, and we can get a parent path. */
  while ((KLI_access(dir, R_OK) != 0) && (valid_path = KLI_path_parent_dir(dir))) {
    /* pass */
  }
  return (valid_path && dir[0]);
}

/**
 * Search for a binary (executable) */
bool KLI_path_program_search(char *fullname, const size_t maxlen, const char *name)
{
#ifdef DEBUG_STRSIZE
  memset(fullname, 0xff, sizeof(*fullname) * maxlen);
#endif
  const char *path;
  bool retval = false;

#ifdef _WIN32
  const char separator = ';';
#else
  const char separator = ':';
#endif

  path = KLI_getenv("PATH");
  if (path) {
    char filename[FILE_MAX];
    const char *temp;

    do {
      temp = strchr(path, separator);
      if (temp) {
        memcpy(filename, path, temp - path);
        filename[temp - path] = 0;
        path = temp + 1;
      } else {
        KLI_strncpy(filename, path, sizeof(filename));
      }

      KLI_path_append(filename, maxlen, name);
      if (
#ifdef _WIN32
        KLI_path_program_extensions_add_win32(filename, maxlen)
#else
        KLI_exists(filename)
#endif
      ) {
        KLI_strncpy(fullname, filename, maxlen);
        retval = true;
        break;
      }
    } while (temp);
  }

  if (retval == false) {
    *fullname = '\0';
  }

  return retval;
}

/**
 * like Python's `os.path.basename()`
 *
 * @return The pointer into @a path string immediately after last slash,
 * or start of @a path if none found. */
const char *KLI_path_basename(const char *path)
{
  const char *const filename = KLI_path_slash_rfind(path);
  return filename ? filename + 1 : path;
}

/**
 * Appends a slash to string if there isn't one there already.
 * Returns the new length of the string. */
int KLI_path_slash_ensure(char *string)
{
  int len = strlen(string);
  if (len == 0 || string[len - 1] != SEP) {
    string[len] = SEP;
    string[len + 1] = '\0';
    return len + 1;
  }
  return len;
}

/**
 * Returns pointer to the rightmost path separator in string. */
const char *KLI_path_slash_rfind(const char *string)
{
  const char *const lfslash = strrchr(string, '/');
  const char *const lbslash = strrchr(string, '\\');

  if (!lfslash) {
    return lbslash;
  }
  if (!lbslash) {
    return lfslash;
  }

  return (lfslash > lbslash) ? lfslash : lbslash;
}

/**
 * Removes the last slash and everything after it to the end of string, if there is one. */
void KLI_path_slash_rstrip(char *string)
{
  int len = strlen(string);
  while (len) {
    if (string[len - 1] == SEP) {
      string[len - 1] = '\0';
      len--;
    } else {
      break;
    }
  }
}

#ifdef _WIN32
/**
 * Tries appending each of the semicolon-separated extensions in the PATHEXT
 * environment variable (Windows-only) onto `name` in turn until such a file is found.
 * Returns success/failure.
 */
bool KLI_path_program_extensions_add_win32(char *name, const size_t maxlen)
{
  bool retval = false;
  fs::file_status type;

  type = KLI_type(name);
  if (KLI_exists(name) || KLI_ISDIR(type)) {
    /* typically 3-5, ".EXE", ".BAT"... etc */
    const int ext_max = 12;
    const char *ext = KLI_getenv("PATHEXT");
    if (ext) {
      const int name_len = strlen(name);
      char *filename = (char *)alloca(name_len + ext_max);
      char *filename_ext;
      const char *ext_next;

      /* null terminated in the loop */
      memcpy(filename, name, name_len);
      filename_ext = filename + name_len;

      do {
        int ext_len;
        ext_next = strchr(ext, ';');
        ext_len = ext_next ? ((ext_next++) - ext) : strlen(ext);

        if (ARCH_LIKELY(ext_len < ext_max)) {
          memcpy(filename_ext, ext, ext_len);
          filename_ext[ext_len] = '\0';

          type = KLI_type(filename);
          if (KLI_exists(name) && (!KLI_ISDIR(type))) {
            retval = true;
            KLI_strncpy(name, filename, maxlen);
            break;
          }
        }
      } while ((ext = ext_next));
    }
  } else {
    retval = true;
  }

  return retval;
}
#endif /* WIN32 */

WABI_NAMESPACE_END