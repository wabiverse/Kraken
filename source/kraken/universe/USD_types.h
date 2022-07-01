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

#include <wabi/usd/usd/stage.h>
#include <wabi/usd/usd/attribute.h>
#include <wabi/usd/usd/collectionAPI.h>
#include <wabi/usd/usd/primRange.h>

WABI_NAMESPACE_BEGIN

struct Main;
struct ReportList;
struct kContext;

/**
 * Macro to use absolute paths to system prims. */
#define WABI_ROOT_NS "WabiAnimationStudios"
#define STAGE(x, ...) SdfPath("/" WABI_ROOT_NS "/" x)
#define STAGE_WABI SdfPath("/" WABI_ROOT_NS)


typedef std::vector<UsdCollectionAPI> UsdCollectionsVector;
typedef std::vector<UsdProperty> UsdPropertyVector;

typedef int (*ObjectValidateFunc)(const UsdPrim &ptr, void *data, int *have_function);
typedef int (*ObjectCallbackFunc)(struct kContext *C,
                                  const UsdPrim &ptr,
                                  void *func,
                                  UsdPropertyVector list);
typedef void (*ObjectFreeFunc)(void *data);
typedef struct KrakenPRIM *(*ObjectRegisterFunc)(struct Main *kmain,
                                                 struct ReportList *reports,
                                                 void *data,
                                                 const char *identifier,
                                                 ObjectValidateFunc validate,
                                                 ObjectCallbackFunc call,
                                                 ObjectFreeFunc free);
typedef void (*ObjectUnregisterFunc)(struct Main *kmain, const UsdPrim &type);
typedef void **(*ObjectInstanceFunc)(struct KrakenPRIM *ptr);

typedef enum PropertyType
{
  PROP_BOOLEAN = 0,
  PROP_INT = 1,
  PROP_FLOAT = 2,
  PROP_STRING = 3,
  PROP_ENUM = 4,
  PROP_POINTER = 5,
  PROP_COLLECTION = 6,
} PropertyType;

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

typedef void (*PropStringGetFunc)(struct KrakenPRIM *ptr, char *value);
typedef int (*PropStringLengthFunc)(struct KrakenPRIM *ptr);
typedef void (*PropStringSetFunc)(struct KrakenPRIM *ptr, const char *value);
typedef int (*PropEnumGetFunc)(struct KrakenPRIM *ptr);
typedef void (*PropStringGetFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop, char *value);
typedef int (*PropStringLengthFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop);
typedef void (*PropStringSetFuncEx)(struct KrakenPRIM *ptr,
                                    struct KrakenPROP *prop,
                                    const char *value);
typedef int (*PropEnumGetFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop);
typedef void (*PropEnumSetFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop, int value);
typedef struct StringPropertySearchVisitParams
{
  /** Text being searched for (never NULL). */
  const char *text;
  /** Additional information to display (optional, may be NULL). */
  const char *info;
} StringPropertySearchVisitParams;
typedef void (*StringPropertySearchVisitFunc)(void *visit_user_data,
                                              const StringPropertySearchVisitParams *params);
typedef void (*StringPropertySearchFunc)(const struct kContext *C,
                                         struct KrakenPRIM *ptr,
                                         struct KrakenPROP *prop,
                                         const char *edit_text,
                                         StringPropertySearchVisitFunc visit_fn,
                                         void *visit_user_data);

typedef enum eStringPropertySearchFlag
{
  PROP_STRING_SEARCH_SUPPORTED = (1 << 0),
  PROP_STRING_SEARCH_SORT = (1 << 1),
  PROP_STRING_SEARCH_SUGGESTION = (1 << 2),
} eStringPropertySearchFlag;

struct KrakenSTAGE : public UsdStageRefPtr
{
  KrakenSTAGE();

  std::vector<struct KrakenPRIM *> structs;
};

WABI_NAMESPACE_END