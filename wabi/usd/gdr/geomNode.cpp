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

#include "wabi/usd/gdr/geomNode.h"
#include "wabi/base/tf/refPtr.h"
#include "wabi/usd/gdr/geomMetadataHelpers.h"
#include "wabi/usd/gdr/geomProperty.h"
#include "wabi/usd/ndr/debugCodes.h"
#include "wabi/wabi.h"

#include <unordered_set>

WABI_NAMESPACE_BEGIN

using GeomMetadataHelpers::StringVal;
using GeomMetadataHelpers::StringVecVal;
using GeomMetadataHelpers::TokenVal;
using GeomMetadataHelpers::TokenVecVal;

TF_DEFINE_PUBLIC_TOKENS(GdrNodeMetadata, GDR_NODE_METADATA_TOKENS);
TF_DEFINE_PUBLIC_TOKENS(GdrNodeContext, GDR_NODE_CONTEXT_TOKENS);
TF_DEFINE_PUBLIC_TOKENS(GdrNodeRole, GDR_NODE_ROLE_TOKENS);

GdrGeomNode::GdrGeomNode(const NdrIdentifier &identifier,
                         const NdrVersion &version,
                         const std::string &name,
                         const TfToken &family,
                         const TfToken &context,
                         const TfToken &sourceType,
                         const std::string &definitionURI,
                         const std::string &implementationURI,
                         NdrPropertyUniquePtrVec &&properties,
                         const NdrTokenMap &metadata,
                         const std::string &sourceCode)
    : NdrNode(identifier,
              version,
              name,
              family,
              context,
              sourceType,
              definitionURI,
              implementationURI,
              std::move(properties),
              metadata,
              sourceCode)
{
  /**
   *  Cast inputs to geom inputs */
  for (const auto &input : _inputs) {
    _geomInputs[input.first] = dynamic_cast<GdrGeomPropertyConstPtr>(input.second);
  }

  /**
   * ... and the same for outputs */
  for (const auto &output : _outputs) {
    _geomOutputs[output.first] = dynamic_cast<GdrGeomPropertyConstPtr>(output.second);
  }

  _InitializePrimvars();
  _PostProcessProperties();

  /**
   * Tokenize metadata. */
  _label       = TokenVal(GdrNodeMetadata->Label, _metadata);
  _category    = TokenVal(GdrNodeMetadata->Category, _metadata);
  _departments = TokenVecVal(GdrNodeMetadata->Departments, _metadata);
  _pages       = _ComputePages();
}

void GdrGeomNode::_PostProcessProperties()
{
  const NdrTokenVec vsNames = GetAllVstructNames();

  /**
   * Declare the input type to be vstruct if it's a vstruct head, and update
   * the default value. */
  for (const TfToken &inputName : _inputNames) {
    NdrTokenVec::const_iterator it = std::find(vsNames.begin(), vsNames.end(), inputName);

    if (it != vsNames.end()) {
      GdrGeomPropertyConstPtr input = _geomInputs.at(inputName);

      const_cast<GdrGeomProperty *>(input)->_type = GdrPropertyTypes->Vstruct;

      const_cast<GdrGeomProperty *>(input)->_defaultValue = VtValue(TfToken());
    }
  }

  /**
   * Declare the output type to be vstruct if it's a vstruct head, and update
   * the default value. */
  for (const TfToken &outputName : _outputNames) {
    NdrTokenVec::const_iterator it = std::find(vsNames.begin(), vsNames.end(), outputName);

    if (it != vsNames.end()) {
      GdrGeomPropertyConstPtr output = _geomOutputs.at(outputName);

      const_cast<GdrGeomProperty *>(output)->_type = GdrPropertyTypes->Vstruct;

      const_cast<GdrGeomProperty *>(output)->_defaultValue = VtValue(TfToken());
    }
  }
}

GdrGeomPropertyConstPtr GdrGeomNode::GetGeomInput(const TfToken &inputName) const
{
  return dynamic_cast<GdrGeomPropertyConstPtr>(NdrNode::GetInput(inputName));
}

GdrGeomPropertyConstPtr GdrGeomNode::GetGeomOutput(const TfToken &outputName) const
{
  return dynamic_cast<GdrGeomPropertyConstPtr>(NdrNode::GetOutput(outputName));
}

NdrTokenVec GdrGeomNode::GetAssetIdentifierInputNames() const
{
  NdrTokenVec result;
  for (const auto &inputName : GetInputNames()) {
    if (auto input = GetGeomInput(inputName)) {
      if (input->IsAssetIdentifier()) {
        result.push_back(input->GetName());
      }
    }
  }
  return result;
}

GdrGeomPropertyConstPtr GdrGeomNode::GetDefaultInput() const
{
  std::vector<GdrGeomPropertyConstPtr> result;
  for (const auto &inputName : GetInputNames()) {
    if (auto input = GetGeomInput(inputName)) {
      if (input->IsDefaultInput()) {
        return input;
      }
    }
  }
  return nullptr;
}

std::string GdrGeomNode::GetHelp() const
{
  return StringVal(GdrNodeMetadata->Help, _metadata);
}

std::string GdrGeomNode::GetImplementationName() const
{
  return StringVal(GdrNodeMetadata->ImplementationName, _metadata, GetName());
}

std::string GdrGeomNode::GetRole() const
{
  return StringVal(GdrNodeMetadata->Role, _metadata, GetName());
}

NdrTokenVec GdrGeomNode::GetPropertyNamesForPage(const std::string &pageName) const
{
  NdrTokenVec propertyNames;

  for (const NdrPropertyUniquePtr &property : _properties) {
    const GdrGeomPropertyConstPtr geomProperty = dynamic_cast<const GdrGeomPropertyConstPtr>(
        property.get());

    if (geomProperty->GetPage() == pageName) {
      propertyNames.push_back(geomProperty->GetName());
    }
  }

  return propertyNames;
}

NdrTokenVec GdrGeomNode::GetAllVstructNames() const
{
  std::unordered_set<std::string> vstructs;

  for (const auto &input : _geomInputs) {
    if (!input.second->IsVStructMember()) {
      continue;
    }

    const TfToken &head = input.second->GetVStructMemberOf();

    if (_geomInputs.count(head)) {
      vstructs.insert(head);
    }
  }

  for (const auto &output : _geomOutputs) {
    if (!output.second->IsVStructMember()) {
      continue;
    }

    const TfToken &head = output.second->GetVStructMemberOf();

    if (_geomOutputs.count(head)) {
      vstructs.insert(head);
    }
  }

  /**
   * Transform the set into a vector */
  return NdrTokenVec(vstructs.begin(), vstructs.end());
}

void GdrGeomNode::_InitializePrimvars()
{
  NdrTokenVec primvars;
  NdrTokenVec primvarNamingProperties;

  /**
   * The "raw" list of primvars contains both ordinary primvars,
   * and the names of properties whose values contain additional
   * primvar names */
  const NdrStringVec rawPrimvars = StringVecVal(GdrNodeMetadata->Primvars, _metadata);

  for (const std::string &primvar : rawPrimvars) {
    if (TfStringStartsWith(primvar, "$")) {
      const std::string propertyName      = TfStringTrimLeft(primvar, "$");
      const GdrGeomPropertyConstPtr input = GetGeomInput(TfToken(propertyName));

      if (input && (input->GetType() == GdrPropertyTypes->String)) {
        primvarNamingProperties.emplace_back(TfToken(std::move(propertyName)));
      }
      else {
        TF_DEBUG(NDR_PARSING)
            .Msg(
                "Found a node [%s] whose metadata "
                "indicates a primvar naming property [%s] "
                "but the property's type is not string; ignoring.",
                GetName().c_str(),
                primvar.c_str());
      }
    }
    else {
      primvars.emplace_back(TfToken(primvar));
    }
  }

  _primvars                = primvars;
  _primvarNamingProperties = primvarNamingProperties;
}

NdrTokenVec GdrGeomNode::_ComputePages() const
{
  NdrTokenVec pages;

  for (const NdrPropertyUniquePtr &property : _properties) {
    auto gdrProperty    = static_cast<GdrGeomPropertyPtr>(property.get());
    const TfToken &page = gdrProperty->GetPage();

    /**
     * Exclude duplicate pages. */
    if (std::find(pages.begin(), pages.end(), page) != pages.end()) {
      continue;
    }

    pages.emplace_back(std::move(page));
  }

  return pages;
}

WABI_NAMESPACE_END
