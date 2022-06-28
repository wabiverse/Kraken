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
 * @file UNI_object.h
 * @ingroup UNI
 * The @a central foundation for @a all data access.
 */

#pragma once

#include "UNI_api.h"
#include "UNI_types.h"

#include <wabi/base/tf/hashmap.h>
#include <wabi/base/tf/notice.h>
#include <wabi/base/tf/singleton.h>
#include <wabi/base/tf/token.h>
#include <wabi/base/tf/weakBase.h>

#include <wabi/usd/usd/prim.h>
#include <wabi/usd/usd/typed.h>

WABI_NAMESPACE_BEGIN

struct PropertyLUXO
{
  TfToken name;
  SdfValueTypeName type;
  SdfVariability variability;
  bool custom;
};

typedef std::vector<PropertyLUXO *> CollectionPropertyLUXO;

typedef struct PointerLUXO *(*StructRefineFunc)(struct PointerLUXO *ptr);

struct ParameterList
{
  void *data;
  struct FunctionLUXO *func;
  int alloc_size;
};

typedef void (*CallFunc)(struct kContext *C,
                         struct ReportList *reports,
                         struct PointerLUXO *ptr,
                         ParameterList *parms);


struct FunctionLUXO
{
  std::vector<FunctionLUXO> cont;

  const char *identifier;

  int flag;

  const char *description;

  CallFunc call;

  PointerLUXO *c_ret;
};

struct PointerLUXO
{
  const char *owner_id;

  /**
   * context (C) */
  void *data;

  PointerLUXO *ptr;
  StructRefineFunc refine;

  SdfPath path;
  const char *identifier;

  UsdPrim type;
  UsdPrim base;

  void *py_type;

  TfNotice notice;

  UsdAttributeVector props;
  CollectionPropertyLUXO collection;

  ObjectRegisterFunc reg;
  ObjectUnregisterFunc unreg;
  ObjectInstanceFunc instance;

  std::vector<PointerLUXO *> functions;
};

WABI_NAMESPACE_END