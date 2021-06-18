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

#ifndef WABI_USD_SDR_SHADER_METADATA_HELPERS_H
#define WABI_USD_SDR_SHADER_METADATA_HELPERS_H

/// \file sdr/shaderMetadataHelpers.h

#include "wabi/base/tf/token.h"
#include "wabi/usd/sdr/api.h"
#include "wabi/usd/sdr/declare.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

/// \namespace ShaderMetadataHelpers
///
/// Various utilities for parsing metadata contained within shaders.
///
namespace ShaderMetadataHelpers
{
/// Determines if the given property in the metadata dictionary has a
/// truthy value. All values are considered to be true except the following
/// (case-insensitive): '0', 'false', and 'f'. The absence of `propName`
/// in the metadata also evaluates to false.
SDR_API
bool IsTruthy(const TfToken &propName, const NdrTokenMap &metadata);

/// Extracts the string value from the given property if it exists,
/// otherwise returns \p defaultValue.
SDR_API
std::string StringVal(const TfToken &propName,
                      const NdrTokenMap &metadata,
                      const std::string &defaultValue = std::string());

/// Extracts the tokenized value from the given property. An empty token is
/// returned if the property does not exist.
SDR_API
TfToken TokenVal(const TfToken &propName,
                 const NdrTokenMap &metadata,
                 const TfToken &defaultValue = TfToken());

/// Extracts a vector of strings from the given property.
SDR_API
NdrStringVec StringVecVal(const TfToken &propName, const NdrTokenMap &metadata);

/// Extracts a vector of tokenized values from the given property. An empty
/// vector is returned if the property does not exist.
SDR_API
NdrTokenVec TokenVecVal(const TfToken &propName, const NdrTokenMap &metadata);

/// Extracts an "options" vector from the given string.
SDR_API
NdrOptionVec OptionVecVal(const std::string &optionStr);

/// Serializes a vector of strings into a string using the pipe character
/// as the delimiter.
SDR_API
std::string CreateStringFromStringVec(const NdrStringVec &stringVec);

/// Determines if the specified property metadata has a widget that
/// indicates the property is an asset identifier.
SDR_API
bool IsPropertyAnAssetIdentifier(const NdrTokenMap &metadata);

/// Determines if the specified property metadata has a 'renderType' that
/// indicates the property should be a SdrPropertyTypes->Terminal
SDR_API
bool IsPropertyATerminal(const NdrTokenMap &metadata);

/// Gets the "role" from metadata if one is provided. Only returns a value
// if it's a valid role as defined by SdrPropertyRole tokens.
SDR_API
TfToken GetRoleFromMetadata(const NdrTokenMap &metadata);
}  // namespace ShaderMetadataHelpers

WABI_NAMESPACE_END

#endif  // WABI_USD_SDR_SHADER_METADATA_HELPERS_H
