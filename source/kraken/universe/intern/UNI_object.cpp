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

#include "UNI_api.h"
#include "UNI_object.h"


WABI_NAMESPACE_BEGIN


KrakenPrim::KrakenPrim(const UsdPrim &prim)
  : UsdTyped(prim),
    notice(TfNotice()),
    type(nullptr),
    data(nullptr),
    base(nullptr)
{}

KrakenPrim::KrakenPrim(const UsdSchemaBase &schemaObj)
  : UsdTyped(schemaObj),
    notice(TfNotice()),
    type(nullptr),
    data(nullptr),
    base(nullptr)
{}

KrakenPrim::~KrakenPrim()
{}


UsdSchemaKind KrakenPrim::GetSchemaKind() const
{
  return KrakenPrim::schemaKind;
}


const TfType &KrakenPrim::GetStaticTfType()
{
  static TfType tfType = TfType::Find<KrakenPrim>();
  return tfType;
}

const TfType &KrakenPrim::GetTfType() const
{
  return GetStaticTfType();
}

bool KrakenPrim::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}


WABI_NAMESPACE_END