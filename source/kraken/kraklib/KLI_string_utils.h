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
 * KRAKEN Library.
 * Gadget Vault.
 */

#pragma once

#include "KLI_api.h"

/* generic strcmp macros */
#if defined(_MSC_VER)
#  define strcasecmp _stricmp
#  define strncasecmp _strnicmp
#endif

/* ------------------------------------------------------------ CLASSIC C STRING UTILITIES ----- */

char *KLI_strdupn(const char *str, const size_t len)
  ATTR_MALLOC ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

char *KLI_strdup(const char *str)
  ATTR_WARN_UNUSED_RESULT ATTR_NONNULL() ATTR_MALLOC;

char *KLI_strdupcat(const char *__restrict str1, const char *__restrict str2)
  ATTR_WARN_UNUSED_RESULT ATTR_NONNULL() ATTR_MALLOC;

int KLI_strcasecmp(const char *s1, const char *s2)
  ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

char *KLI_strcasestr(const char *s, const char *find)
  ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

int KLI_strncasecmp(const char *s1, const char *s2, size_t len)
  ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

int KLI_strcasecmp_natural(const char *s1, const char *s2)
  ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

void KLI_str_replace_char(char *str, char src, char dst)
  ATTR_NONNULL();

size_t KLI_split_name_num(char *left, int *nr, const char *name, const char delim);

char *KLI_strncpy(char *__restrict dst, const char *__restrict src, const size_t maxncpy)
  ATTR_NONNULL();

size_t KLI_strncpy_rlen(char *__restrict dst, const char *__restrict src, const size_t maxncpy)
  ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

size_t KLI_strncpy_utf8_rlen(char *__restrict dst, const char *__restrict src, size_t maxncpy)
  ATTR_NONNULL();


size_t KLI_strncpy_wchar_as_utf8(char *__restrict dst,
                                 const wchar_t *__restrict src,
                                 const size_t maxncpy)
  ATTR_NONNULL();

int KLI_str_utf8_size(const char *p)
  ATTR_NONNULL();

int KLI_str_utf8_size_safe(const char *p)
  ATTR_NONNULL();

size_t KLI_str_utf8_from_unicode(uint c, char *outbuf);

size_t KLI_strnlen(const char *s, const size_t maxlen)
  ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

size_t KLI_vsnprintf(char *__restrict buffer, size_t maxncpy, const char *__restrict format, va_list arg)
  ATTR_PRINTF_FORMAT(3, 0);

size_t KLI_vsnprintf_rlen(char *__restrict buffer,
                          size_t maxncpy,
                          const char *__restrict format,
                          va_list arg)
  ATTR_PRINTF_FORMAT(3, 0);

size_t KLI_snprintf(char *__restrict dst, size_t maxncpy, const char *__restrict format, ...)
  ATTR_NONNULL(1, 3) ATTR_PRINTF_FORMAT(3, 4);

size_t KLI_snprintf_rlen(char *__restrict dst, size_t maxncpy, const char *__restrict format, ...)
  ATTR_NONNULL(1, 3) ATTR_PRINTF_FORMAT(3, 4);

size_t KLI_str_escape(char *__restrict dst, const char *__restrict src, const size_t dst_maxncpy)
  ATTR_NONNULL();

size_t KLI_str_unescape(char *__restrict dst, const char *__restrict src, const size_t src_maxncpy)
  ATTR_NONNULL();

typedef bool (*UniquenameCheckCallback)(void *arg, const char *name);

bool KLI_uniquename(std::vector<void *> list, void *vlink, const char *defname, char delim, int name_offset, size_t name_len);
bool KLI_uniquename_cb(UniquenameCheckCallback unique_check,
                       void *arg,
                       const char *defname,
                       char delim,
                       char *name,
                       size_t name_len);

/* Join strings, return newly allocated string. */
char *KLI_string_join_array(char *result,
                            size_t result_len,
                            const char *strings[],
                            uint strings_len)
  ATTR_NONNULL();

/* Take multiple arguments, pass as (array, length). */
#define KLI_string_join(result, result_len, ...) \
  KLI_string_join_array(result, result_len, ((const char *[]){__VA_ARGS__}), VA_NARGS_COUNT(__VA_ARGS__))

#define STRNCPY(dst, src) KLI_strncpy(dst, src, ARRAY_SIZE(dst))
#define STRNCPY_RLEN(dst, src) KLI_strncpy_rlen(dst, src, ARRAY_SIZE(dst))
#define SNPRINTF(dst, format, ...) KLI_snprintf(dst, ARRAY_SIZE(dst), format, __VA_ARGS__)
#define SNPRINTF_RLEN(dst, format, ...) KLI_snprintf_rlen(dst, ARRAY_SIZE(dst), format, __VA_ARGS__)
#define STR_CONCAT(dst, len, suffix) len += KLI_strncpy_rlen(dst + len, suffix, ARRAY_SIZE(dst) - len)
#define STR_CONCATF(dst, len, format, ...) len += KLI_snprintf_rlen(dst + len, ARRAY_SIZE(dst) - len, format, __VA_ARGS__)

#define STREQ(a, b) (strcmp(a, b) == 0)
#define STRCASEEQ(a, b) (strcasecmp(a, b) == 0)
#define STREQLEN(a, b, n) (strncmp(a, b, n) == 0)
#define STRCASEEQLEN(a, b, n) (strncasecmp(a, b, n) == 0)

#define STRPREFIX(a, b) (strncmp((a), (b), strlen(b)) == 0)

#define UTF8_COMPUTE(Char, Mask, Len, Err) \
  if (Char < 128) \
  { \
    Len = 1; \
    Mask = 0x7f; \
  } \
  else if ((Char & 0xe0) == 0xc0) \
  { \
    Len = 2; \
    Mask = 0x1f; \
  } \
  else if ((Char & 0xf0) == 0xe0) \
  { \
    Len = 3; \
    Mask = 0x0f; \
  } \
  else if ((Char & 0xf8) == 0xf0) \
  { \
    Len = 4; \
    Mask = 0x07; \
  } \
  else if ((Char & 0xfc) == 0xf8) \
  { \
    Len = 5; \
    Mask = 0x03; \
  } \
  else if ((Char & 0xfe) == 0xfc) \
  { \
    Len = 6; \
    Mask = 0x01; \
  } \
  else \
  { \
    Len = Err; /* -1 is the typical error value or 1 to skip */ \
  } \
  (void)0

/* ------------------------------------------------------ MODERN CXX STD::STRING UTILITIES ----- */

std::string KLI_str_CapSpaceAmmender(std::string str);
std::string KLI_str_UpperCamel(std::string str);
