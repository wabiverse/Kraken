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
 * UTF-Convert.
 * Making Windows Unicode Compliant.
 */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

/* --------------------------------- WINDOWS ONLY ----- */

#ifdef WIN32

int umkdir(const char *pathname);

#endif /* WIN32 */

/* ---------------------------------------------------- */

/** Error occurs when required parameter is missing. */
#define UTF_ERROR_NULL_IN (1 << 0)
/** Error if character is in illegal UTF range. */
#define UTF_ERROR_ILLCHAR (1 << 1)
/** Passed size is to small. It gives legal string with character missing at the end. */
#define UTF_ERROR_SMALL (1 << 2)
/** Error if sequence is broken and doesn't finish. */
#define UTF_ERROR_ILLSEQ (1 << 3)

size_t count_utf_8_from_16(const wchar_t *string16);
int conv_utf_16_to_8(const wchar_t *in16, char *out8, size_t size8);

size_t count_utf_16_from_8(const char *string8);
int conv_utf_8_to_16(const char *in8, wchar_t *out16, size_t size16);

char *alloc_utf_8_from_16(const wchar_t *in16, size_t add);
wchar_t *alloc_utf16_from_8(const char *in8, size_t add);

#define UTF16_ENCODE(in8str) \
  if (1) {                   \
  wchar_t *in8str##_16 = alloc_utf16_from_8((const char *)in8str, 0)

#define UTF16_UN_ENCODE(in8str) \
  free(in8str##_16);            \
  }                             \
  (void)0