//
// Copyright 2016 Pixar
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
#include "wabi/imaging/glf/diagnostic.h"

#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hd/types.h"
#include "wabi/imaging/hdPh/bufferArrayRange.h"
#include "wabi/imaging/hdPh/bufferResource.h"
#include "wabi/imaging/hdPh/copyComputation.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/tokens.h"

#include "wabi/imaging/hgi/blitCmds.h"
#include "wabi/imaging/hgi/blitCmdsOps.h"

#include "wabi/imaging/hf/perfLog.h"

WABI_NAMESPACE_BEGIN

HdPhCopyComputationGPU::HdPhCopyComputationGPU(HdBufferArrayRangeSharedPtr const &src,
                                               TfToken const &name)
  : _src(src),
    _name(name)
{}

void HdPhCopyComputationGPU::Execute(HdBufferArrayRangeSharedPtr const &range_,
                                     HdResourceRegistry *resourceRegistry)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  HdPhBufferArrayRangeSharedPtr srcRange = std::static_pointer_cast<HdPhBufferArrayRange>(_src);
  HdPhBufferArrayRangeSharedPtr dstRange = std::static_pointer_cast<HdPhBufferArrayRange>(range_);

  HdPhBufferResourceSharedPtr srcRes = srcRange->GetResource(_name);
  HdPhBufferResourceSharedPtr dstRes = dstRange->GetResource(_name);

  if (!TF_VERIFY(srcRes)) {
    return;
  }
  if (!TF_VERIFY(dstRes)) {
    return;
  }

  int srcResSize = HdDataSizeOfTupleType(srcRes->GetTupleType()) * srcRange->GetNumElements();
  int dstResSize = HdDataSizeOfTupleType(dstRes->GetTupleType()) * dstRange->GetNumElements();

  if (!TF_VERIFY(srcResSize <= dstResSize)) {
    // The number of elements in the BAR *can* differ during migration.
    // One example is during mesh refinement when migration is necessary,
    // and we copy only the unrefined data over.
    TF_CODING_ERROR(
      "Migration error for %s: Source resource (%d) size is "
      "larger than destination resource size (%d)\n",
      _name.GetText(),
      srcResSize,
      dstResSize);
    return;
  }

  size_t readOffset = srcRange->GetByteOffset(_name) + srcRes->GetOffset();
  size_t writeOffset = dstRange->GetByteOffset(_name) + dstRes->GetOffset();
  size_t copySize = srcResSize;

  // Unfortunately at the time the copy computation is added, we don't
  // know if the source buffer has 0 length.  So we can get here with
  // a zero sized copy.
  if (srcResSize > 0) {

    // If the buffer's have 0 size, resources for them would not have
    // be allocated, so the check for resource allocation has been moved
    // until after the copy size check.

    if (!TF_VERIFY(srcRes->GetHandle())) {
      return;
    }
    if (!TF_VERIFY(dstRes->GetHandle())) {
      return;
    }

    HD_PERF_COUNTER_INCR(HdPhPerfTokens->copyBufferGpuToGpu);

    HdPhResourceRegistry *hdPhResourceRegistry = static_cast<HdPhResourceRegistry *>(
      resourceRegistry);

    HgiBufferGpuToGpuOp blitOp;
    blitOp.gpuSourceBuffer = srcRes->GetHandle();
    blitOp.gpuDestinationBuffer = dstRes->GetHandle();
    blitOp.sourceByteOffset = readOffset;
    blitOp.byteSize = copySize;
    blitOp.destinationByteOffset = writeOffset;

    HgiBlitCmds *blitCmds = hdPhResourceRegistry->GetGlobalBlitCmds();
    blitCmds->CopyBufferGpuToGpu(blitOp);
  }

  GLF_POST_PENDING_GL_ERRORS();
}

int HdPhCopyComputationGPU::GetNumOutputElements() const
{
  return _src->GetNumElements();
}

void HdPhCopyComputationGPU::GetBufferSpecs(HdBufferSpecVector *specs) const
{
  HdPhBufferArrayRangeSharedPtr srcRange = std::static_pointer_cast<HdPhBufferArrayRange>(_src);

  specs->emplace_back(_name, srcRange->GetResource(_name)->GetTupleType());
}

WABI_NAMESPACE_END
