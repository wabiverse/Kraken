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

WABI_NAMESPACE_BEGIN

struct UniverseObject : public UsdPrim, public TfRefBase, public TfWeakBase
{
  SdfPath path;

  /**
   * Notifier to MsgBus */
  TfNotice notice = TfNotice();

  const char *identifier;

  struct UniverseObject *type;
  void *data;

  UsdAttributeVector props;

  /**
   * Object this is derived from */
  struct UniverseObject *base;

  /**
   * Function to register/unregister subclasses */
  ObjectRegisterFunc reg;
  ObjectUnregisterFunc unreg;

  virtual ~UniverseObject()
  {}

  inline UniverseObject();
};

UniverseObject::UniverseObject()
  : notice(TfNotice()),
    type(nullptr),
    data(nullptr),
    base(nullptr)
{}

struct UniverseProperty
{
  TfToken name;
  SdfValueTypeName type;
  SdfVariability variability;
  bool custom;
};

typedef UniverseObject PointerUNI;

typedef std::vector<UniverseProperty> UniversePropertyVec;

WABI_NAMESPACE_END