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
///
/// \file sdf/fileIO.cpp

#include "wabi/wabi.h"

#include "wabi/usd/sdf/fileIO.h"

#include "wabi/usd/sdf/attributeSpec.h"
#include "wabi/usd/sdf/fileIO_Common.h"
#include "wabi/usd/sdf/primSpec.h"
#include "wabi/usd/sdf/relationshipSpec.h"
#include "wabi/usd/sdf/variantSetSpec.h"
#include "wabi/usd/sdf/variantSpec.h"

#include "wabi/base/tf/stringUtils.h"

#include <ostream>

WABI_NAMESPACE_BEGIN

#if AR_VERSION != 1

Sdf_StreamWritableAsset::~Sdf_StreamWritableAsset() = default;

#endif

bool Sdf_WriteToStream(const SdfSpec &baseSpec, std::ostream &o, size_t indent)
{
  Sdf_TextOutput out(o);

  const SdfSpecType type = baseSpec.GetSpecType();

  switch (type) {
    case SdfSpecTypePrim: {
      SdfPrimSpec spec = Sdf_CastAccess::CastSpec<SdfPrimSpec, SdfSpec>(baseSpec);
      return Sdf_WritePrim(spec, out, indent);
    }
    case SdfSpecTypeAttribute: {
      SdfAttributeSpec spec = Sdf_CastAccess::CastSpec<SdfAttributeSpec, SdfSpec>(baseSpec);
      return Sdf_WriteAttribute(spec, out, indent);
    }
    case SdfSpecTypeRelationship: {
      SdfRelationshipSpec spec = Sdf_CastAccess::CastSpec<SdfRelationshipSpec, SdfSpec>(baseSpec);
      return Sdf_WriteRelationship(spec, out, indent);
    }
    case SdfSpecTypeVariantSet: {
      SdfVariantSetSpec spec = Sdf_CastAccess::CastSpec<SdfVariantSetSpec, SdfSpec>(baseSpec);
      return Sdf_WriteVariantSet(spec, out, indent);
    }
    case SdfSpecTypeVariant: {
      SdfVariantSpec spec = Sdf_CastAccess::CastSpec<SdfVariantSpec, SdfSpec>(baseSpec);
      return Sdf_WriteVariant(spec, out, indent);
    }
    default:
      break;
  }

  TF_CODING_ERROR("Cannot write spec of type %s to stream", TfStringify(type).c_str());
  return false;
}

WABI_NAMESPACE_END
