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
 * Universe.
 * Set the Stage.
 */

#include <wabi/usd/usd/attribute.h>

WABI_NAMESPACE_BEGIN

struct Main;
struct ReportList;
struct kContext;

typedef int (*ObjectValidateFunc)(const UsdPrim &ptr, void *data, int *have_function);
typedef int (*ObjectCallbackFunc)(struct kContext *C,
                                  const UsdPrim &ptr,
                                  void *func,
                                  UsdAttributeVector list);
typedef void (*ObjectFreeFunc)(void *data);
typedef struct PointerLUXO *(*ObjectRegisterFunc)(struct Main *kmain,
                                                  struct ReportList *reports,
                                                  void *data,
                                                  const char *identifier,
                                                  ObjectValidateFunc validate,
                                                  ObjectCallbackFunc call,
                                                  ObjectFreeFunc free);
typedef void (*ObjectUnregisterFunc)(struct Main *kmain, const UsdPrim &type);
typedef void **(*ObjectInstanceFunc)(struct PointerLUXO *ptr);

typedef enum FunctionFlag
{
  FUNC_USE_SELF_ID = (1 << 11),
  FUNC_NO_SELF = (1 << 0),
  FUNC_USE_SELF_TYPE = (1 << 1),
  FUNC_USE_MAIN = (1 << 2),
  FUNC_USE_CONTEXT = (1 << 3),
  FUNC_USE_REPORTS = (1 << 4),
  FUNC_REGISTER = (1 << 5),
  FUNC_REGISTER_OPTIONAL = FUNC_REGISTER | (1 << 6),
  FUNC_ALLOW_WRITE = (1 << 12),
  FUNC_RUNTIME = (1 << 9),
} FunctionFlag;

struct KrakenPIXAR
{
  std::vector<struct PointerLUXO *> structs = {NULL, NULL};
};

WABI_NAMESPACE_END