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
#include "wabi/imaging/glf/contextCaps.h"

#include "wabi/imaging/hdPh/bufferResource.h"
#include "wabi/imaging/hdPh/glUtils.h"
#include "wabi/imaging/hdPh/interleavedMemoryManager.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/tokens.h"

#include "wabi/imaging/hgi/blitCmds.h"
#include "wabi/imaging/hgi/blitCmdsOps.h"
#include "wabi/imaging/hgi/buffer.h"
#include "wabi/imaging/hgi/hgi.h"

#include "wabi/base/arch/hash.h"
#include "wabi/base/tf/diagnostic.h"
#include "wabi/base/tf/enum.h"
#include "wabi/base/tf/iterator.h"

#include "wabi/imaging/hd/perfLog.h"
#include "wabi/imaging/hd/tokens.h"

#include "wabi/imaging/hf/perfLog.h"

#include <vector>

WABI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  HdPhInterleavedMemoryManager
// ---------------------------------------------------------------------------
HdBufferArrayRangeSharedPtr HdPhInterleavedMemoryManager::CreateBufferArrayRange()
{
  return std::make_shared<_StripedInterleavedBufferRange>(_resourceRegistry);
}

/// Returns the buffer specs from a given buffer array
HdBufferSpecVector HdPhInterleavedMemoryManager::GetBufferSpecs(
  HdBufferArraySharedPtr const &bufferArray) const
{
  _StripedInterleavedBufferSharedPtr bufferArray_ = std::static_pointer_cast<_StripedInterleavedBuffer>(
    bufferArray);
  return bufferArray_->GetBufferSpecs();
}

/// Returns the size of the GPU memory used by the passed buffer array
size_t HdPhInterleavedMemoryManager::GetResourceAllocation(HdBufferArraySharedPtr const &bufferArray,
                                                           VtDictionary &result) const
{
  std::set<uint64_t> idSet;
  size_t gpuMemoryUsed = 0;

  _StripedInterleavedBufferSharedPtr bufferArray_ = std::static_pointer_cast<_StripedInterleavedBuffer>(
    bufferArray);

  TF_FOR_ALL (resIt, bufferArray_->GetResources())
  {
    HdPhBufferResourceSharedPtr const &resource = resIt->second;

    HgiBufferHandle buffer = resource->GetHandle();

    // XXX avoid double counting of resources shared within a buffer
    uint64_t id = buffer ? buffer->GetRawResource() : 0;
    if (idSet.count(id) == 0)
    {
      idSet.insert(id);

      std::string const &role = resource->GetRole().GetString();
      size_t size = size_t(resource->GetSize());

      if (result.count(role))
      {
        size_t currentSize = result[role].Get<size_t>();
        result[role] = VtValue(currentSize + size);
      }
      else
      {
        result[role] = VtValue(size);
      }

      gpuMemoryUsed += size;
    }
  }

  return gpuMemoryUsed;
}

// ---------------------------------------------------------------------------
//  HdPhInterleavedUBOMemoryManager
// ---------------------------------------------------------------------------
HdBufferArraySharedPtr HdPhInterleavedUBOMemoryManager::CreateBufferArray(
  TfToken const &role,
  HdBufferSpecVector const &bufferSpecs,
  HdBufferArrayUsageHint usageHint)
{
  const GlfContextCaps &caps = GlfContextCaps::GetInstance();

  return std::make_shared<HdPhInterleavedMemoryManager::_StripedInterleavedBuffer>(
    this,
    _resourceRegistry,
    role,
    bufferSpecs,
    usageHint,
    caps.uniformBufferOffsetAlignment,
    /*structAlignment=*/sizeof(float) * 4,
    caps.maxUniformBlockSize,
    HdPerfTokens->garbageCollectedUbo);
}

HdAggregationStrategy::AggregationId HdPhInterleavedUBOMemoryManager::ComputeAggregationId(
  HdBufferSpecVector const &bufferSpecs,
  HdBufferArrayUsageHint usageHint) const
{
  static size_t salt = ArchHash(__FUNCTION__, sizeof(__FUNCTION__));
  size_t result = salt;
  for (HdBufferSpec const &spec : bufferSpecs)
  {
    boost::hash_combine(result, spec.Hash());
  }
  boost::hash_combine(result, usageHint.value);

  // promote to size_t
  return (AggregationId)result;
}

// ---------------------------------------------------------------------------
//  HdPhInterleavedSSBOMemoryManager
// ---------------------------------------------------------------------------
HdBufferArraySharedPtr HdPhInterleavedSSBOMemoryManager::CreateBufferArray(
  TfToken const &role,
  HdBufferSpecVector const &bufferSpecs,
  HdBufferArrayUsageHint usageHint)
{
  const GlfContextCaps &caps = GlfContextCaps::GetInstance();

  return std::make_shared<HdPhInterleavedMemoryManager::_StripedInterleavedBuffer>(
    this,
    _resourceRegistry,
    role,
    bufferSpecs,
    usageHint,
    /*bufferOffsetAlignment=*/0,
    /*structAlignment=*/0,
    caps.maxShaderStorageBlockSize,
    HdPerfTokens->garbageCollectedSsbo);
}

HdAggregationStrategy::AggregationId HdPhInterleavedSSBOMemoryManager::ComputeAggregationId(
  HdBufferSpecVector const &bufferSpecs,
  HdBufferArrayUsageHint usageHint) const
{
  static size_t salt = ArchHash(__FUNCTION__, sizeof(__FUNCTION__));
  size_t result = salt;
  for (HdBufferSpec const &spec : bufferSpecs)
  {
    boost::hash_combine(result, spec.Hash());
  }
  boost::hash_combine(result, usageHint.value);

  return result;
}

// ---------------------------------------------------------------------------
//  _StripedInterleavedBuffer
// ---------------------------------------------------------------------------

static inline int _ComputePadding(int alignment, int currentOffset)
{
  return ((alignment - (currentOffset & (alignment - 1))) & (alignment - 1));
}

static inline int _ComputeAlignment(HdTupleType tupleType)
{
  const HdType componentType = HdGetComponentType(tupleType.type);
  const int numComponents = HdGetComponentCount(tupleType.type);
  const int componentSize = HdDataSizeOfType(componentType);

  // This is simplified to treat arrays of int and floats
  // as vectors. The padding rules state that if we have
  // an array of 2 ints, it would get aligned to the size
  // of a vec4, where as a vec2 of ints or floats is aligned
  // to the size of a vec2. Since we don't know if something is
  // an array or vector, we are treating them as vectors.
  //
  // XXX:Arrays: Now that we do know whether a value is an array
  // or vector, we can update this to do the right thing.

  // Matrices are treated as an array of vec4s, so the
  // max num components we are looking at is 4
  int alignComponents = std::min(numComponents, 4);

  // single elements and vec2's are allowed, but
  // vec3's get rounded up to vec4's
  if (alignComponents == 3)
  {
    alignComponents = 4;
  }

  return componentSize * alignComponents;
}

HdPhInterleavedMemoryManager::_StripedInterleavedBuffer::_StripedInterleavedBuffer(
  HdPhInterleavedMemoryManager *mgr,
  HdPhResourceRegistry *resourceRegistry,
  TfToken const &role,
  HdBufferSpecVector const &bufferSpecs,
  HdBufferArrayUsageHint usageHint,
  int bufferOffsetAlignment = 0,
  int structAlignment = 0,
  size_t maxSize = 0,
  TfToken const &garbageCollectionPerfToken = HdPerfTokens->garbageCollectedUbo)
  : HdBufferArray(role, garbageCollectionPerfToken, usageHint),
    _manager(mgr),
    _resourceRegistry(resourceRegistry),
    _needsCompaction(false),
    _stride(0),
    _bufferOffsetAlignment(bufferOffsetAlignment),
    _maxSize(maxSize)

{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  /*
     interleaved uniform buffer layout (for example)

              .--range["color"].offset
              v
    .--------------------------------------------------.
    | Xf      : Color      || Xf       : Color   || ...|
    '--------------------------------------------------'
     ^------- stride ------^
     ^---- one element ----^
  */

  /*
   do std140/std430 packing (GL spec section 7.6.2.2)
    When using the "std430" storage layout, shader storage
    blocks will be laid out in buffer storage identically to uniform and
    shader storage blocks using the "std140" layout, except that the base
    alignment of arrays of scalars and vectors in rule (4) and of structures
    in rule (9) are not rounded up a multiple of the base alignment of a vec4.
   */

  TF_FOR_ALL (it, bufferSpecs)
  {
    // Figure out the alignment we need for this type of data
    int alignment = _ComputeAlignment(it->tupleType);
    _stride += _ComputePadding(alignment, _stride);

    // We need to save the max alignment size for later because the
    // stride for our struct needs to be aligned to this
    structAlignment = std::max(structAlignment, alignment);

    _stride += HdDataSizeOfTupleType(it->tupleType);
  }

  // Our struct stride needs to be aligned to the max alignment needed within
  // our struct.
  _stride += _ComputePadding(structAlignment, _stride);

  // and also aligned if bufferOffsetAlignment exists (for UBO binding)
  if (_bufferOffsetAlignment > 0)
  {
    _stride += _ComputePadding(_bufferOffsetAlignment, _stride);
  }

  TF_VERIFY(_stride > 0);

  TF_DEBUG_MSG(HD_BUFFER_ARRAY_INFO, "Create interleaved buffer array: stride = %d\n", _stride);

  // populate BufferResources, interleaved
  int offset = 0;
  TF_FOR_ALL (it, bufferSpecs)
  {
    // Figure out alignment for this data member
    int alignment = _ComputeAlignment(it->tupleType);
    // Add any needed padding to fixup alignment
    offset += _ComputePadding(alignment, offset);

    _AddResource(it->name, it->tupleType, offset, _stride);

    TF_DEBUG_MSG(
      HD_BUFFER_ARRAY_INFO, "  %s : offset = %d, alignment = %d\n", it->name.GetText(), offset, alignment);

    offset += HdDataSizeOfTupleType(it->tupleType);
  }

  _SetMaxNumRanges(_maxSize / _stride);

  TF_VERIFY(_stride + offset);
}

HdPhBufferResourceSharedPtr HdPhInterleavedMemoryManager::_StripedInterleavedBuffer::_AddResource(
  TfToken const &name,
  HdTupleType tupleType,
  int offset,
  int stride)
{
  HD_TRACE_FUNCTION();

  if (TfDebug::IsEnabled(HD_SAFE_MODE))
  {
    // duplication check
    HdPhBufferResourceSharedPtr bufferRes = GetResource(name);
    if (!TF_VERIFY(!bufferRes))
    {
      return bufferRes;
    }
  }

  HdPhBufferResourceSharedPtr bufferRes = std::make_shared<HdPhBufferResource>(
    GetRole(), tupleType, offset, stride);

  _resourceList.emplace_back(name, bufferRes);
  return bufferRes;
}

HdPhInterleavedMemoryManager::_StripedInterleavedBuffer::~_StripedInterleavedBuffer()
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  // invalidate buffer array ranges in range list
  // (these ranges may still be held by drawItems)
  size_t rangeCount = GetRangeCount();
  for (size_t rangeIdx = 0; rangeIdx < rangeCount; ++rangeIdx)
  {
    _StripedInterleavedBufferRangeSharedPtr range = _GetRangeSharedPtr(rangeIdx);

    if (range)
    {
      range->Invalidate();
    }
  }
}

bool HdPhInterleavedMemoryManager::_StripedInterleavedBuffer::GarbageCollect()
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  if (_needsCompaction)
  {
    RemoveUnusedRanges();

    std::vector<HdBufferArrayRangeSharedPtr> ranges;
    size_t rangeCount = GetRangeCount();
    ranges.reserve(rangeCount);
    for (size_t i = 0; i < rangeCount; ++i)
    {
      HdBufferArrayRangeSharedPtr range = GetRange(i).lock();
      if (range)
        ranges.push_back(range);
    }
    Reallocate(ranges, shared_from_this());
  }

  if (GetRangeCount() == 0)
  {
    _DeallocateResources();
    return true;
  }

  return false;
}

void HdPhInterleavedMemoryManager::_StripedInterleavedBuffer::Reallocate(
  std::vector<HdBufferArrayRangeSharedPtr> const &ranges,
  HdBufferArraySharedPtr const &curRangeOwner)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  HgiBlitCmds *blitCmds = _resourceRegistry->GetGlobalBlitCmds();
  blitCmds->PushDebugGroup(__ARCH_PRETTY_FUNCTION__);

  HD_PERF_COUNTER_INCR(HdPerfTokens->vboRelocated);

  // Calculate element count
  size_t elementCount = 0;
  TF_FOR_ALL (it, ranges)
  {
    HdBufferArrayRangeSharedPtr const &range = *it;
    if (!range)
    {
      TF_CODING_ERROR("Expired range found in the reallocation list");
    }
    elementCount += (*it)->GetNumElements();
  }
  size_t totalSize = elementCount * _stride;

  // update range list (should be done before early exit)
  _SetRangeList(ranges);

  // resize each BufferResource
  // all HdBufferSources are sharing same VBO

  // allocate new one
  // curBuf and oldBuf will be different when we are adopting ranges
  // from another buffer array.
  HgiBufferHandle &oldBuf = GetResources().begin()->second->GetHandle();

  _StripedInterleavedBufferSharedPtr curRangeOwner_ = std::static_pointer_cast<_StripedInterleavedBuffer>(
    curRangeOwner);

  HgiBufferHandle const &curBuf = curRangeOwner_->GetResources().begin()->second->GetHandle();
  HgiBufferHandle newBuf;

  Hgi *hgi = _resourceRegistry->GetHgi();

  // Skip buffers of zero size.
  if (totalSize > 0)
  {
    HgiBufferDesc bufDesc;
    bufDesc.byteSize = totalSize;
    bufDesc.usage = HgiBufferUsageUniform;
    newBuf = hgi->CreateBuffer(bufDesc);
  }

  // if old and new buffer exist, copy unchanged data
  if (curBuf && newBuf)
  {
    int index = 0;

    size_t rangeCount = GetRangeCount();

    // pre-pass to combine consecutive buffer range relocation
    HdPhBufferRelocator relocator(curBuf, newBuf);
    for (size_t rangeIdx = 0; rangeIdx < rangeCount; ++rangeIdx)
    {
      _StripedInterleavedBufferRangeSharedPtr range = _GetRangeSharedPtr(rangeIdx);

      if (!range)
      {
        TF_CODING_ERROR(
          "_StripedInterleavedBufferRange expired "
          "unexpectedly.");
        continue;
      }
      int oldIndex = range->GetElementOffset();
      if (oldIndex >= 0)
      {
        // copy old data
        ptrdiff_t readOffset = oldIndex * _stride;
        ptrdiff_t writeOffset = index * _stride;
        ptrdiff_t copySize = _stride * range->GetNumElements();

        relocator.AddRange(readOffset, writeOffset, copySize);
      }

      range->SetIndex(index);
      index += range->GetNumElements();
    }

    // buffer copy
    relocator.Commit(blitCmds);
  }
  else
  {
    // just set index
    int index = 0;

    size_t rangeCount = GetRangeCount();
    for (size_t rangeIdx = 0; rangeIdx < rangeCount; ++rangeIdx)
    {
      _StripedInterleavedBufferRangeSharedPtr range = _GetRangeSharedPtr(rangeIdx);
      if (!range)
      {
        TF_CODING_ERROR(
          "_StripedInterleavedBufferRange expired "
          "unexpectedly.");
        continue;
      }

      range->SetIndex(index);
      index += range->GetNumElements();
    }
  }
  if (oldBuf)
  {
    // delete old buffer
    hgi->DestroyBuffer(&oldBuf);
  }

  // update allocation to all buffer resources
  TF_FOR_ALL (it, GetResources())
  {
    it->second->SetAllocation(newBuf, totalSize);
  }

  blitCmds->PopDebugGroup();

  _needsReallocation = false;
  _needsCompaction = false;

  // increment version to rebuild dispatch buffers.
  IncrementVersion();
}

void HdPhInterleavedMemoryManager::_StripedInterleavedBuffer::_DeallocateResources()
{
  HdPhBufferResourceSharedPtr resource = GetResource();
  if (resource)
  {
    _resourceRegistry->GetHgi()->DestroyBuffer(&resource->GetHandle());
  }
}

void HdPhInterleavedMemoryManager::_StripedInterleavedBuffer::DebugDump(std::ostream &out) const
{
  out << "  HdPhInterleavedMemoryManager\n";
  out << "    Range entries " << GetRangeCount() << ":\n";

  size_t rangeCount = GetRangeCount();
  for (size_t rangeIdx = 0; rangeIdx < rangeCount; ++rangeIdx)
  {
    _StripedInterleavedBufferRangeSharedPtr range = _GetRangeSharedPtr(rangeIdx);

    if (range)
    {
      out << "      " << rangeIdx << *range;
    }
  }
}

HdPhBufferResourceSharedPtr HdPhInterleavedMemoryManager::_StripedInterleavedBuffer::GetResource() const
{
  HD_TRACE_FUNCTION();

  if (_resourceList.empty())
    return HdPhBufferResourceSharedPtr();

  if (TfDebug::IsEnabled(HD_SAFE_MODE))
  {
    // make sure this buffer array has only one resource.
    HgiBufferHandle const &buffer = _resourceList.begin()->second->GetHandle();
    TF_FOR_ALL (it, _resourceList)
    {
      if (it->second->GetHandle() != buffer)
      {
        TF_CODING_ERROR(
          "GetResource(void) called on"
          "HdBufferArray having multiple GL resources");
      }
    }
  }

  // returns the first item
  return _resourceList.begin()->second;
}

HdPhBufferResourceSharedPtr HdPhInterleavedMemoryManager::_StripedInterleavedBuffer::GetResource(
  TfToken const &name)
{
  HD_TRACE_FUNCTION();

  // linear search.
  // The number of buffer resources should be small (<10 or so).
  for (HdPhBufferResourceNamedList::iterator it = _resourceList.begin(); it != _resourceList.end(); ++it)
  {
    if (it->first == name)
      return it->second;
  }
  return HdPhBufferResourceSharedPtr();
}

HdBufferSpecVector HdPhInterleavedMemoryManager::_StripedInterleavedBuffer::GetBufferSpecs() const
{
  HdBufferSpecVector result;
  result.reserve(_resourceList.size());
  TF_FOR_ALL (it, _resourceList)
  {
    result.emplace_back(it->first, it->second->GetTupleType());
  }
  return result;
}

// ---------------------------------------------------------------------------
//  _StripedInterleavedBufferRange
// ---------------------------------------------------------------------------
HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::~_StripedInterleavedBufferRange()
{
  // Notify that hosting buffer array needs to be garbage collected.
  //
  // Don't do any substantial work here.
  //
  if (_stripedBuffer)
  {
    _stripedBuffer->SetNeedsCompaction();
  }
}

bool HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::IsAssigned() const
{
  return (_stripedBuffer != nullptr);
}

bool HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::IsImmutable() const
{
  return (_stripedBuffer != nullptr) && _stripedBuffer->IsImmutable();
}

bool HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::Resize(int numElements)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  if (!TF_VERIFY(_stripedBuffer))
    return false;

  // interleaved BAR never needs to be resized, since numElements in buffer
  // resources is always 1. Note that the arg numElements of this function
  // could be more than 1 for static array.
  // ignore Resize request.

  // XXX: this could be a problem if a client allows to change the array size
  //      dynamically -- e.g. instancer nesting level changes.
  //
  return false;
}

HdPhInterleavedMemoryManager::_BufferFlushListEntry::_BufferFlushListEntry(HgiBufferHandle const &buf,
                                                                           uint64_t s,
                                                                           uint64_t e)
  : buffer(buf),
    start(s),
    end(e)
{}

void HdPhInterleavedMemoryManager::StageBufferCopy(HgiBufferCpuToGpuOp const &copyOp)
{
  if (copyOp.byteSize == 0 || !copyOp.cpuSourceBuffer || !copyOp.gpuDestinationBuffer)
  {
    return;
  }

  HgiBlitCmds *blitCmds = _resourceRegistry->GetGlobalBlitCmds();

  // When the to-be-copied data is 'large' doing the extra memcpy into the
  // stating buffer to avoid many small GPU buffer upload can be more
  // expensive than just submitting the CPU to GPU copy operation directly.
  // The value of 'queueThreshold' is estimated (when is the extra memcpy
  // into the staging buffer slower than immediately issuing a gpu upload)
  static const int queueThreshold = 512 * 1024;
  if (copyOp.byteSize > queueThreshold)
  {
    blitCmds->CopyBufferCpuToGpu(copyOp);
    return;
  }

  // Place the data into the staging buffer.
  uint8_t *const cpuStaging = static_cast<uint8_t *>(copyOp.gpuDestinationBuffer->GetCPUStagingAddress());
  uint8_t const *const srcData = static_cast<uint8_t const *>(copyOp.cpuSourceBuffer) +
                                 copyOp.sourceByteOffset;
  memcpy(cpuStaging + copyOp.destinationByteOffset, srcData, copyOp.byteSize);

  auto const &it = _queuedBuffers.find(copyOp.gpuDestinationBuffer.Get());
  if (it != _queuedBuffers.end())
  {
    _BufferFlushListEntry &bufferEntry = it->second;
    if (copyOp.destinationByteOffset == bufferEntry.end)
    {
      // Accumulate the copy
      bufferEntry.end += copyOp.byteSize;
    }
    else
    {
      // This buffer copy doesn't contiguously extend the queued copy
      // Submit the accumulated work to date
      HgiBufferCpuToGpuOp op;
      op.cpuSourceBuffer = cpuStaging;
      op.sourceByteOffset = bufferEntry.start;
      op.gpuDestinationBuffer = copyOp.gpuDestinationBuffer;
      op.destinationByteOffset = bufferEntry.start;
      op.byteSize = bufferEntry.end - bufferEntry.start;
      blitCmds->CopyBufferCpuToGpu(op);

      // Update this entry for our new pending copy
      bufferEntry.start = copyOp.destinationByteOffset;
      bufferEntry.end = copyOp.destinationByteOffset + copyOp.byteSize;
    }
  }
  else
  {
    uint64_t const start = copyOp.destinationByteOffset;
    uint64_t const end = copyOp.destinationByteOffset + copyOp.byteSize;
    _queuedBuffers.emplace(copyOp.gpuDestinationBuffer.Get(),
                           _BufferFlushListEntry(copyOp.gpuDestinationBuffer, start, end));
  }
}

void HdPhInterleavedMemoryManager::Flush()
{
  HgiBlitCmds *blitCmds = _resourceRegistry->GetGlobalBlitCmds();

  HgiBufferCpuToGpuOp op;
  for (auto &copy : _queuedBuffers)
  {
    _BufferFlushListEntry const &entry = copy.second;
    op.cpuSourceBuffer = entry.buffer->GetCPUStagingAddress();
    op.sourceByteOffset = entry.start;
    op.gpuDestinationBuffer = entry.buffer;
    op.destinationByteOffset = entry.start;
    op.byteSize = entry.end - entry.start;
    blitCmds->CopyBufferCpuToGpu(op);
  }
  _queuedBuffers.clear();
}

void HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::CopyData(
  HdBufferSourceSharedPtr const &bufferSource)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  if (!TF_VERIFY(_stripedBuffer))
    return;

  HdPhBufferResourceSharedPtr VBO = _stripedBuffer->GetResource(bufferSource->GetName());

  if (!VBO || !VBO->GetHandle())
  {
    TF_CODING_ERROR("VBO doesn't exist for %s", bufferSource->GetName().GetText());
    return;
  }

  // overrun check
  // XXX:Arrays:  Note that we only check tuple type here, not arity.
  // This code allows N-tuples and N-element arrays to be interchanged.
  // It would seem better to have upstream buffers adjust their tuple
  // arity as needed.
  if (!TF_VERIFY(bufferSource->GetTupleType().type == VBO->GetTupleType().type,
                 "'%s': (%s (%i) x %zu) != (%s (%i) x %zu)\n",
                 bufferSource->GetName().GetText(),
                 TfEnum::GetName(bufferSource->GetTupleType().type).c_str(),
                 bufferSource->GetTupleType().type,
                 bufferSource->GetTupleType().count,
                 TfEnum::GetName(VBO->GetTupleType().type).c_str(),
                 VBO->GetTupleType().type,
                 VBO->GetTupleType().count))
  {
    return;
  }

  int vboStride = VBO->GetStride();
  size_t vboOffset = VBO->GetOffset() + vboStride * _index;
  int dataSize = HdDataSizeOfTupleType(VBO->GetTupleType());
  const unsigned char *data = (const unsigned char *)bufferSource->GetData();

  HgiBufferCpuToGpuOp blitOp;
  blitOp.gpuDestinationBuffer = VBO->GetHandle();
  blitOp.sourceByteOffset = 0;
  blitOp.byteSize = dataSize;

  for (size_t i = 0; i < _numElements; ++i)
  {
    blitOp.cpuSourceBuffer = data;

    blitOp.destinationByteOffset = vboOffset;
    _stripedBuffer->GetManager()->StageBufferCopy(blitOp);

    vboOffset += vboStride;
    data += dataSize;
  }

  HD_PERF_COUNTER_ADD(HdPhPerfTokens->copyBufferCpuToGpu, (double)_numElements);
}

VtValue HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::ReadData(TfToken const &name) const
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  VtValue result;
  if (!TF_VERIFY(_stripedBuffer))
    return result;

  HdPhBufferResourceSharedPtr VBO = _stripedBuffer->GetResource(name);

  if (!VBO || !VBO->GetHandle())
  {
    TF_CODING_ERROR("VBO doesn't exist for %s", name.GetText());
    return result;
  }

  result = HdPhGLUtils::ReadBuffer(VBO->GetHandle()->GetRawResource(),
                                   VBO->GetTupleType(),
                                   VBO->GetOffset() + VBO->GetStride() * _index,
                                   VBO->GetStride(),
                                   _numElements);

  return result;
}

size_t HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::GetMaxNumElements() const
{
  return _stripedBuffer->GetMaxNumElements();
}

HdBufferArrayUsageHint HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::GetUsageHint() const
{
  if (!TF_VERIFY(_stripedBuffer))
  {
    return HdBufferArrayUsageHint();
  }

  return _stripedBuffer->GetUsageHint();
}

HdPhBufferResourceSharedPtr HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::GetResource() const
{
  if (!TF_VERIFY(_stripedBuffer))
    return HdPhBufferResourceSharedPtr();

  return _stripedBuffer->GetResource();
}

HdPhBufferResourceSharedPtr HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::GetResource(
  TfToken const &name)
{
  if (!TF_VERIFY(_stripedBuffer))
    return HdPhBufferResourceSharedPtr();

  // don't use GetResource(void) as a shortcut even an interleaved buffer
  // is sharing one underlying GL resource. We may need an appropriate
  // offset depending on name.
  return _stripedBuffer->GetResource(name);
}

HdPhBufferResourceNamedList const &HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::
  GetResources() const
{
  if (!TF_VERIFY(_stripedBuffer))
  {
    static HdPhBufferResourceNamedList empty;
    return empty;
  }
  return _stripedBuffer->GetResources();
}

void HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::SetBufferArray(HdBufferArray *bufferArray)
{
  _stripedBuffer = static_cast<_StripedInterleavedBuffer *>(bufferArray);
}

void HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::DebugDump(std::ostream &out) const
{
  out << "[StripedIBR] index = " << _index << "\n";
}

const void *HdPhInterleavedMemoryManager::_StripedInterleavedBufferRange::_GetAggregation() const
{
  return _stripedBuffer;
}

WABI_NAMESPACE_END
