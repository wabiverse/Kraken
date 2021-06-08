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
#ifndef WABI_IMAGING_HD_PH_INTERLEAVED_MEMORY_MANAGER_H
#define WABI_IMAGING_HD_PH_INTERLEAVED_MEMORY_MANAGER_H

#include "wabi/base/tf/mallocTag.h"
#include "wabi/base/tf/token.h"
#include "wabi/imaging/hd/bufferArray.h"
#include "wabi/imaging/hd/bufferSource.h"
#include "wabi/imaging/hd/bufferSpec.h"
#include "wabi/imaging/hd/resource.h"
#include "wabi/imaging/hd/strategyBase.h"
#include "wabi/imaging/hd/tokens.h"
#include "wabi/imaging/hd/version.h"
#include "wabi/imaging/hdPh/api.h"
#include "wabi/imaging/hdPh/bufferArrayRange.h"
#include "wabi/imaging/hgi/buffer.h"
#include "wabi/wabi.h"

#include <list>
#include <memory>
#include <unordered_map>

WABI_NAMESPACE_BEGIN

class HdPhResourceRegistry;
struct HgiBufferCpuToGpuOp;

/// \class HdPhInterleavedMemoryManager
///
/// Interleaved memory manager (base class).
///
class HdPhInterleavedMemoryManager : public HdAggregationStrategy {
 public:
  /// Copy new data from CPU into staging buffer.
  /// This reduces the amount of GPU copy commands we emit by first writing
  /// to the CPU staging area of the buffer and only flushing it to the GPU
  /// when we write to a non-consecutive area of a buffer.
  void StageBufferCopy(HgiBufferCpuToGpuOp const &copyOp);

  /// Flush the staging buffer to GPU.
  /// Copy the new buffer data from staging area to GPU.
  void Flush() override;

 protected:
  class _StripedInterleavedBuffer;

  // BufferFlushListEntry lets use accumulate writes into the same GPU buffer
  // into CPU staging buffers before flushing to GPU.
  class _BufferFlushListEntry {
   public:
    _BufferFlushListEntry(HgiBufferHandle const &buf, uint64_t start, uint64_t end);

    HgiBufferHandle buffer;
    uint64_t start;
    uint64_t end;
  };

  using _BufferFlushMap = std::unordered_map<class HgiBuffer *, _BufferFlushListEntry>;

  /// specialized buffer array range
  class _StripedInterleavedBufferRange : public HdPhBufferArrayRange {
   public:
    /// Constructor.
    _StripedInterleavedBufferRange(HdPhResourceRegistry *resourceRegistry)
        : HdPhBufferArrayRange(resourceRegistry),
          _stripedBuffer(nullptr),
          _index(NOT_ALLOCATED),
          _numElements(1)
    {}

    /// Destructor.
    HDPH_API
    ~_StripedInterleavedBufferRange() override;

    /// Returns true if this range is valid
    bool IsValid() const override
    {
      // note: a range is valid even its index is NOT_ALLOCATED.
      return (bool)_stripedBuffer;
    }

    /// Returns true is the range has been assigned to a buffer
    HDPH_API
    bool IsAssigned() const override;

    /// Returns true if this range is marked as immutable.
    bool IsImmutable() const override;

    /// Resize memory area for this range. Returns true if it causes container
    /// buffer reallocation.
    HDPH_API
    bool Resize(int numElements) override;

    /// Copy source data into buffer
    HDPH_API
    void CopyData(HdBufferSourceSharedPtr const &bufferSource) override;

    /// Read back the buffer content
    HDPH_API
    VtValue ReadData(TfToken const &name) const override;

    /// Returns the offset at which this range begins in the underlying
    /// buffer array in terms of elements.
    int GetElementOffset() const override
    {
      return _index;
    }

    /// Returns the byte offset at which this range begins in the underlying
    /// buffer array for the given resource.
    int GetByteOffset(TfToken const &resourceName) const override
    {
      TF_UNUSED(resourceName);
      if (!TF_VERIFY(_stripedBuffer) || !TF_VERIFY(_index != NOT_ALLOCATED))
        return 0;
      return _stripedBuffer->GetStride() * _index;
    }

    /// Returns the number of elements
    size_t GetNumElements() const override
    {
      return _numElements;
    }

    /// Returns the version of the buffer array.
    size_t GetVersion() const override
    {
      return _stripedBuffer->GetVersion();
    }

    /// Increment the version of the buffer array.
    void IncrementVersion() override
    {
      _stripedBuffer->IncrementVersion();
    }

    /// Returns the max number of elements
    HDPH_API
    size_t GetMaxNumElements() const override;

    /// Returns the usage hint from the underlying buffer array
    HDPH_API
    HdBufferArrayUsageHint GetUsageHint() const override;

    /// Returns the GPU resource. If the buffer array contains more than one
    /// resource, this method raises a coding error.
    HDPH_API
    HdPhBufferResourceSharedPtr GetResource() const override;

    /// Returns the named GPU resource.
    HDPH_API
    HdPhBufferResourceSharedPtr GetResource(TfToken const &name) override;

    /// Returns the list of all named GPU resources for this bufferArrayRange.
    HDPH_API
    HdPhBufferResourceNamedList const &GetResources() const override;

    /// Sets the buffer array associated with this buffer;
    HDPH_API
    void SetBufferArray(HdBufferArray *bufferArray) override;

    /// Debug dump
    HDPH_API
    void DebugDump(std::ostream &out) const override;

    /// Set the relative offset for this range.
    void SetIndex(int index)
    {
      _index = index;
    }

    /// Make this range invalid
    void Invalidate()
    {
      _stripedBuffer = nullptr;
    }

   protected:
    /// Returns the aggregation container
    HDPH_API
    const void *_GetAggregation() const override;

   private:
    enum { NOT_ALLOCATED = -1 };
    _StripedInterleavedBuffer *_stripedBuffer;
    int _index;
    size_t _numElements;
  };

  using _StripedInterleavedBufferSharedPtr      = std::shared_ptr<_StripedInterleavedBuffer>;
  using _StripedInterleavedBufferRangeSharedPtr = std::shared_ptr<_StripedInterleavedBufferRange>;
  using _StripedInterleavedBufferRangePtr       = std::weak_ptr<_StripedInterleavedBufferRange>;

  /// striped buffer
  class _StripedInterleavedBuffer : public HdBufferArray {
   public:
    /// Constructor.
    HDPH_API
    _StripedInterleavedBuffer(HdPhInterleavedMemoryManager *mgr,
                              HdPhResourceRegistry *resourceRegistry,
                              TfToken const &role,
                              HdBufferSpecVector const &bufferSpecs,
                              HdBufferArrayUsageHint usageHint,
                              int bufferOffsetAlignment,
                              int structAlignment,
                              size_t maxSize,
                              TfToken const &garbageCollectionPerfToken);

    /// Destructor. It invalidates _rangeList
    HDPH_API
    virtual ~_StripedInterleavedBuffer();

    /// perform compaction if necessary, returns true if it becomes empty.
    HDPH_API
    virtual bool GarbageCollect();

    /// Debug output
    HDPH_API
    virtual void DebugDump(std::ostream &out) const;

    /// Performs reallocation.
    /// GLX context has to be set when calling this function.
    HDPH_API
    virtual void Reallocate(std::vector<HdBufferArrayRangeSharedPtr> const &ranges,
                            HdBufferArraySharedPtr const &curRangeOwner);

    /// Mark to perform reallocation on Reallocate()
    void SetNeedsReallocation()
    {
      _needsReallocation = true;
    }

    /// Mark to perform compaction on GarbageCollect()
    void SetNeedsCompaction()
    {
      _needsCompaction = true;
    }

    /// Returns the stride.
    int GetStride() const
    {
      return _stride;
    }

    /// TODO: We need to distinguish between the primvar types here, we should
    /// tag each HdBufferSource and HdBufferResource with Constant, Uniform,
    /// Varying, Vertex, or FaceVarying and provide accessors for the specific
    /// buffer types.

    /// Returns the GPU resource. If the buffer array contains more than one
    /// resource, this method raises a coding error.
    HDPH_API
    HdPhBufferResourceSharedPtr GetResource() const;

    /// Returns the named GPU resource. This method returns the first found
    /// resource. In HD_SAFE_MODE it checks all underlying GL buffers
    /// in _resourceMap and raises a coding error if there are more than
    /// one GL buffers exist.
    HDPH_API
    HdPhBufferResourceSharedPtr GetResource(TfToken const &name);

    /// Returns the list of all named GPU resources for this bufferArray.
    HdPhBufferResourceNamedList const &GetResources() const
    {
      return _resourceList;
    }

    /// Reconstructs the bufferspecs and returns it (for buffer splitting)
    HDPH_API
    HdBufferSpecVector GetBufferSpecs() const;

    HdPhInterleavedMemoryManager *GetManager() const
    {
      return _manager;
    }

   protected:
    HDPH_API
    void _DeallocateResources();

    /// Adds a new, named GPU resource and returns it.
    HDPH_API
    HdPhBufferResourceSharedPtr _AddResource(TfToken const &name,
                                             HdTupleType tupleType,
                                             int offset,
                                             int stride);

   private:
    HdPhInterleavedMemoryManager *_manager;
    HdPhResourceRegistry *const _resourceRegistry;
    bool _needsCompaction;
    int _stride;
    int _bufferOffsetAlignment;  // ranged binding offset alignment
    size_t _maxSize;             // maximum size of single buffer

    HdPhBufferResourceNamedList _resourceList;

    _StripedInterleavedBufferRangeSharedPtr _GetRangeSharedPtr(size_t idx) const
    {
      return std::static_pointer_cast<_StripedInterleavedBufferRange>(GetRange(idx).lock());
    }
  };

  HdPhInterleavedMemoryManager(HdPhResourceRegistry *resourceRegistry)
      : _resourceRegistry(resourceRegistry)
  {}

  /// Factory for creating HdBufferArrayRange
  HdBufferArrayRangeSharedPtr CreateBufferArrayRange() override;

  /// Returns the buffer specs from a given buffer array
  HdBufferSpecVector GetBufferSpecs(HdBufferArraySharedPtr const &bufferArray) const override;

  /// Returns the size of the GPU memory used by the passed buffer array
  size_t GetResourceAllocation(HdBufferArraySharedPtr const &bufferArray,
                               VtDictionary &result) const override;

  HdPhResourceRegistry *const _resourceRegistry;
  _BufferFlushMap _queuedBuffers;
};

class HdPhInterleavedUBOMemoryManager : public HdPhInterleavedMemoryManager {
 public:
  HdPhInterleavedUBOMemoryManager(HdPhResourceRegistry *resourceRegistry)
      : HdPhInterleavedMemoryManager(resourceRegistry)
  {}

  /// Factory for creating HdBufferArray managed by
  /// HdPhVBOMemoryManager aggregation.
  HDPH_API
  virtual HdBufferArraySharedPtr CreateBufferArray(TfToken const &role,
                                                   HdBufferSpecVector const &bufferSpecs,
                                                   HdBufferArrayUsageHint usageHint);

  /// Returns id for given bufferSpecs to be used for aggregation
  HDPH_API
  virtual AggregationId ComputeAggregationId(HdBufferSpecVector const &bufferSpecs,
                                             HdBufferArrayUsageHint usageHint) const;
};

class HdPhInterleavedSSBOMemoryManager : public HdPhInterleavedMemoryManager {
 public:
  HdPhInterleavedSSBOMemoryManager(HdPhResourceRegistry *resourceRegistry)
      : HdPhInterleavedMemoryManager(resourceRegistry)
  {}

  /// Factory for creating HdBufferArray managed by
  /// HdPhVBOMemoryManager aggregation.
  HDPH_API
  virtual HdBufferArraySharedPtr CreateBufferArray(TfToken const &role,
                                                   HdBufferSpecVector const &bufferSpecs,
                                                   HdBufferArrayUsageHint usageHint);

  /// Returns id for given bufferSpecs to be used for aggregation
  HDPH_API
  virtual AggregationId ComputeAggregationId(HdBufferSpecVector const &bufferSpecs,
                                             HdBufferArrayUsageHint usageHint) const;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HD_PH_INTERLEAVED_MEMORY_MANAGER_H
