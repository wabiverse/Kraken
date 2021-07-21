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

struct KrakenPrim : public UsdTyped
{
  explicit KrakenPrim(const UsdPrim &prim = UsdPrim());
  explicit KrakenPrim(const UsdSchemaBase &schemaObj);
  virtual ~KrakenPrim();

  static bool RegisterPrimInitFromPlugins(KrakenPrim *prim);

  static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;

  /** This scenes active stage. */
  SdfPath path;
  TfNotice notice;

  const char *identifier;
  struct KrakenPrim *type;
  void *data;

  UsdAttributeVector props;
  CollectionPropertyLUXO collection;

  struct KrakenPrim *base;

  ObjectRegisterFunc reg;
  ObjectUnregisterFunc unreg;
  ObjectInstanceFunc instance;

 protected:
  UsdSchemaKind GetSchemaKind() const override;

 private:
  friend class UsdSchemaRegistry;

  static const TfType &GetStaticTfType();
  static bool IsTypedSchema();
  const TfType &GetTfType() const override;
};

typedef KrakenPrim PointerLUXO;

WABI_NAMESPACE_END