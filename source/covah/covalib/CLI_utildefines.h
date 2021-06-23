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
 * COVAH Library.
 * Gadget Vault.
 */

#pragma once

/* std */
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

/* wabi */
#include <wabi/wabi.h>

/* base */
#include <wabi/base/arch/export.h>

/* covalib */
#include "CLI_compiler_attrs.h"

/**
 *  Macro to convert a value to string in the pre-processor: */
#define STRINGIFY_ARG(x) "" #x
#define STRINGIFY_APPEND(a, b) "" a #b
#define STRINGIFY(x) STRINGIFY_APPEND("", x)

#define CLI_assert(a) ((void)0)

/**
 * UNUSED macro, for function argument */
#if defined(__GNUC__) || defined(__clang__)
#  define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_##x
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_##x
#else
#  define UNUSED_FUNCTION(x) UNUSED_##x
#endif

#define IFACE_(msgid) msgid

#if defined(__GNUC__) && !defined(__cplusplus) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#  define ARRAY_SIZE(arr) \
    ((sizeof(struct { int isnt_array : ((const void *)&(arr) == &(arr)[0]); }) * 0) + \
     (sizeof(arr) / sizeof(*(arr))))
#else
#  define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))
#endif

#ifdef __GNUC__
#  define ATTR_NONNULL(args...) __attribute__((nonnull(args)))
#else
#  define ATTR_NONNULL(...)
#endif

#define FIND_TOKEN(i) (TfToken::Find(i))

#define STRINGALL(x) TfStringify(x)
#define CHARALL(x) TfStringify(x).c_str()

#define CONCAT(a, b) TfStringCatPaths(a, b).c_str()
#define STRCAT(a, b) TfStringCatPaths(a, b)

#define CHARSTR(a) a.c_str()
