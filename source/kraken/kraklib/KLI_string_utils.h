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

#pragma once

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#include <stdarg.h>

#include "KLI_compiler_attrs.h"
#include "KLI_utildefines.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ListBase;

/**
 * Take multiple arguments, pass as (array, length).
 */
#define KLI_string_join(result, result_len, ...) \
  KLI_string_join_array( \
      result, result_len, ((const char *[]){__VA_ARGS__}), VA_NARGS_COUNT(__VA_ARGS__))
#define KLI_string_joinN(...) \
  KLI_string_join_arrayN(((const char *[]){__VA_ARGS__}), VA_NARGS_COUNT(__VA_ARGS__))
#define KLI_string_join_by_sep_charN(sep, ...) \
  KLI_string_join_array_by_sep_charN( \
      sep, ((const char *[]){__VA_ARGS__}), VA_NARGS_COUNT(__VA_ARGS__))
#define KLI_string_join_by_sep_char_with_tableN(sep, table, ...) \
  KLI_string_join_array_by_sep_char_with_tableN( \
      sep, table, ((const char *[]){__VA_ARGS__}), VA_NARGS_COUNT(__VA_ARGS__))

#ifdef __cplusplus
}
#endif