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
#include "wabi/imaging/hdPh/extCompGpuPrimvarBufferSource.h"

#include "wabi/imaging/hd/bufferSpec.h"
#include "wabi/imaging/hd/tokens.h"

WABI_NAMESPACE_BEGIN

HdPhExtCompGpuPrimvarBufferSource::HdPhExtCompGpuPrimvarBufferSource(TfToken const &name,
                                                                     HdTupleType const &valueType,
                                                                     int numElements,
                                                                     SdfPath const &compId)
    : HdNullBufferSource(),
      _name(name),
      _tupleType(valueType),
      _numElements(numElements),
      _compId(compId)
{}

/* virtual */
TfToken const &HdPhExtCompGpuPrimvarBufferSource::GetName() const
{
  return _name;
}

/* virtual */
size_t HdPhExtCompGpuPrimvarBufferSource::ComputeHash() const
{
  // Simply return a hash based on the computation and primvar names,
  // instead of hashing the contents of the inputs to the computation.
  // This effectively disables primvar sharing when using computed primvars.
  size_t hash = 0;
  boost::hash_combine(hash, _compId);
  boost::hash_combine(hash, _name);
  return hash;
}

/* virtual */
bool HdPhExtCompGpuPrimvarBufferSource::Resolve()
{
  if (!_TryLock())
    return false;
  _SetResolved();
  return true;
}

/* virtual */
bool HdPhExtCompGpuPrimvarBufferSource::_CheckValid() const
{
  return true;
}

/* virtual */
size_t HdPhExtCompGpuPrimvarBufferSource::GetNumElements() const
{
  return _numElements;
}

/* virtual */
HdTupleType HdPhExtCompGpuPrimvarBufferSource::GetTupleType() const
{
  return _tupleType;
}

/* virtual */
void HdPhExtCompGpuPrimvarBufferSource::GetBufferSpecs(HdBufferSpecVector *specs) const
{
  specs->emplace_back(_name, _tupleType);
}

WABI_NAMESPACE_END
