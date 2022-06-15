/*
 * Copyright 2021 Pixar. All Rights Reserved.
 *
 * Portions of this file are derived from original work by Pixar
 * distributed with Universal Scene Description, a project of the
 * Academy Software Foundation (ASWF). https://www.aswf.io/
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *    names, trademarks, service marks, or product names of the Licensor
 *    and its affiliates, except as required to comply with Section 4(c)
 *    of the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the Apache License for the
 * specific language governing permissions and limitations under the
 * Apache License.
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */
#ifndef WABI_USD_USD_LUX_LIGHT_DEF_PARSER_H
#define WABI_USD_USD_LUX_LIGHT_DEF_PARSER_H

#include "wabi/usd/usdLux/api.h"
#include "wabi/wabi.h"

#include "wabi/usd/ndr/declare.h"
#include "wabi/usd/ndr/parserPlugin.h"

WABI_NAMESPACE_BEGIN

/// \class UsdLux_LightDefParserPlugin
///
/// Parses shader definitions from the registered prim definitions for
/// UsdLuxLight and UsdLuxLightFilter derived schema classes.
///
class UsdLux_LightDefParserPlugin : public NdrParserPlugin
{
 public:

  USDLUX_API
  UsdLux_LightDefParserPlugin() = default;

  USDLUX_API
  ~UsdLux_LightDefParserPlugin() override = default;

  USDLUX_API
  NdrNodeUniquePtr Parse(const NdrNodeDiscoveryResult &discoveryResult) override;

  USDLUX_API
  const NdrTokenVec &GetDiscoveryTypes() const override;

  USDLUX_API
  const TfToken &GetSourceType() const override;

 private:

  // The discovery plugin needs to match the source type and discovery types
  // that instances of this parser returns when discovering nodes.
  friend class UsdLux_DiscoveryPlugin;
  static const TfToken &_GetSourceType();
  static const TfToken &_GetDiscoveryType();
};

WABI_NAMESPACE_END

#endif  // WABI_USD_USD_LUX_LIGHT_DEF_PARSER_H
