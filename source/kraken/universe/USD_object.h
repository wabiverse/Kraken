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
 * @file USD_object.h
 * @ingroup UNI
 * The @a central foundation for @a all data access.
 */

#pragma once

#include "USD_api.h"
#include "USD_types.h"

#include "KLI_string_utils.h"

#include <wabi/base/tf/hashmap.h>
#include <wabi/base/tf/notice.h>
#include <wabi/base/tf/singleton.h>
#include <wabi/base/tf/token.h>
#include <wabi/base/tf/weakBase.h>

#include <wabi/usd/usd/prim.h>
#include <wabi/usd/usd/typed.h>
#include <wabi/usd/usd/collectionAPI.h>

WABI_NAMESPACE_BEGIN

struct KrakenPROP : UsdProperty
{
  KrakenPROP(const UsdProperty &prop = UsdProperty()) : UsdProperty(prop) {}

  TfToken name;
  PropertyType type;
};

typedef std::vector<KrakenPROP *> PropertyVectorLUXO;

struct KrakenPROPString
{
  KrakenPROP property;

  PropStringGetFunc get;
  PropStringLengthFunc length;
  PropStringSetFunc set;

  PropStringGetFuncEx get_ex;
  PropStringLengthFuncEx length_ex;
  PropStringSetFuncEx set_ex;

  StringPropertySearchFunc search;
  eStringPropertySearchFlag search_flag;

  int maxlength;

  const char *defaultvalue;
};

struct ParameterList
{
  void *data;
  struct KrakenFUNC *func;
  int alloc_size;
};

typedef struct KrakenPRIM *(*StructRefineFunc)(struct KrakenPRIM *ptr);

typedef void (*CallFunc)(struct kContext *C,
                         struct ReportList *reports,
                         struct KrakenPRIM *ptr,
                         ParameterList *parms);


struct KrakenFUNC
{
  std::vector<KrakenFUNC> cont;

  const char *identifier;

  int flag;

  const char *description;

  CallFunc call;

  KrakenPRIM *c_ret;
};

struct KrakenPRIM : public UsdPrim
{
  KrakenPRIM(const UsdPrim &prim = UsdPrim())
    : UsdPrim(prim),
      owner_id(IsValid() ? GetParent().GetName().GetText() : NULL),
      identifier(!GetName().IsEmpty() ? GetName().GetText() : NULL),
      collection(UsdCollectionAPI())
  {}

  const char *owner_id;

  /**
   * context (C) */
  void *data;

  KrakenPRIM *type;
  KrakenPRIM *base;
  StructRefineFunc refine;

  const char *identifier;

  void *py_type;

  PropertyVectorLUXO props;
  UsdCollectionAPI collection;

  ObjectRegisterFunc reg;
  ObjectUnregisterFunc unreg;
  ObjectInstanceFunc instance;

  std::vector<KrakenPRIM *> functions;
};

WABI_NAMESPACE_END