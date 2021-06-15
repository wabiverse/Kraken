//
// Copyright 2017 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "wabi/imaging/hdPh/bufferArrayRange.h"
#include "wabi/imaging/hd/bufferSpec.h"
#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hdPh/bufferResource.h"

WABI_NAMESPACE_BEGIN

HdPhBufferArrayRange::HdPhBufferArrayRange(HdPhResourceRegistry *resourceRegistry)
  : _resourceRegistry(resourceRegistry)
{}

HdPhBufferArrayRange::~HdPhBufferArrayRange()
{}

void HdPhBufferArrayRange::GetBufferSpecs(HdBufferSpecVector *specs) const
{
  HD_TRACE_FUNCTION();

  HdPhBufferResourceNamedList const &resources = GetResources();

  TF_FOR_ALL(it, resources)
  {
    specs->emplace_back(it->first, it->second->GetTupleType());
  }
}

HdPhResourceRegistry *HdPhBufferArrayRange::GetResourceRegistry()
{
  return _resourceRegistry;
}

std::ostream &operator<<(std::ostream &out, const HdPhBufferArrayRange &self)
{
  // call virtual
  self.DebugDump(out);
  return out;
}

void HdPhBufferArrayRangeContainer::Set(int index, HdPhBufferArrayRangeSharedPtr const &range)
{
  HD_TRACE_FUNCTION();

  if (index < 0) {
    TF_CODING_ERROR("Index negative in HdPhBufferArrayRangeContainer::Set()");
    return;
  }

  if (static_cast<size_t>(index) >= _ranges.size()) {
    HD_PERF_COUNTER_INCR(HdPerfTokens->bufferArrayRangeContainerResized);
    _ranges.resize(index + 1);
  }
  _ranges[index] = range;
}

HdPhBufferArrayRangeSharedPtr const &HdPhBufferArrayRangeContainer::Get(int index) const
{
  if (index < 0 || static_cast<size_t>(index) >= _ranges.size()) {
    // out of range access is not an errorneous path.
    // (i.e. element/instance bars can be null if not exists)
    static HdPhBufferArrayRangeSharedPtr empty;
    return empty;
  }
  return _ranges[index];
}

WABI_NAMESPACE_END
