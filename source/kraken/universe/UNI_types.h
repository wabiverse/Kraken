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
struct KrakenPrim;
struct PropertyLUXO;
struct kContext;

typedef struct KrakenPrim KrakenPrim;

typedef int (*ObjectValidateFunc)(struct KrakenPrim *ptr, void *data, int *have_function);
typedef int (*ObjectCallbackFunc)(struct kContext *C,
                                  struct KrakenPrim *ptr,
                                  void *func,
                                  UsdAttributeVector list);
typedef void (*ObjectFreeFunc)(void *data);
typedef struct KrakenPrim *(*ObjectRegisterFunc)(struct Main *kmain,
                                                 struct ReportList *reports,
                                                 void *data,
                                                 const char *identifier,
                                                 ObjectValidateFunc validate,
                                                 ObjectCallbackFunc call,
                                                 ObjectFreeFunc free);
typedef void (*ObjectUnregisterFunc)(struct Main *kmain, struct KrakenPrim *type);
typedef void **(*ObjectInstanceFunc)(struct KrakenPrim *ptr);

/**
 * Kraken UNI
 *
 * Root UNI data structure that lists all prim types. */

struct KrakenUNI
{
  std::vector<struct KrakenPrim *> objects;
};

WABI_NAMESPACE_END