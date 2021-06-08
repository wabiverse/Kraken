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
#include "wabi/imaging/hdPh/rprimUtils.h"
#include "wabi/wabi.h"

#include "wabi/imaging/hdPh/debugCodes.h"
#include "wabi/imaging/hdPh/drawItem.h"
#include "wabi/imaging/hdPh/instancer.h"
#include "wabi/imaging/hdPh/material.h"
#include "wabi/imaging/hdPh/mixinShader.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/shaderCode.h"

#include "wabi/imaging/hd/bufferArrayRange.h"
#include "wabi/imaging/hd/bufferSource.h"
#include "wabi/imaging/hd/bufferSpec.h"
#include "wabi/imaging/hd/computation.h"
#include "wabi/imaging/hd/renderDelegate.h"
#include "wabi/imaging/hd/renderIndex.h"
#include "wabi/imaging/hd/rprim.h"
#include "wabi/imaging/hd/rprimSharedData.h"
#include "wabi/imaging/hd/types.h"
#include "wabi/imaging/hd/vtBufferSource.h"

#include "wabi/imaging/glf/contextCaps.h"
#include "wabi/imaging/hf/diagnostic.h"

#include "wabi/imaging/hio/glslfx.h"

#include "wabi/base/arch/hash.h"
#include "wabi/base/tf/envSetting.h"

#include <algorithm>

WABI_NAMESPACE_BEGIN

TF_DEFINE_ENV_SETTING(HDPH_ENABLE_SHARED_VERTEX_PRIMVAR, 1, "Enable sharing of vertex primvar");

// -----------------------------------------------------------------------------
// Primvar descriptor filtering utilities
// -----------------------------------------------------------------------------
static bool _IsEnabledPrimvarFiltering(HdPhDrawItem const *drawItem)
{
  HdPhShaderCodeSharedPtr materialShader = drawItem->GetMaterialShader();
  return materialShader && materialShader->IsEnabledPrimvarFiltering();
}

static TfTokenVector _GetFilterNames(HdRprim const *prim,
                                     HdPhDrawItem const *drawItem,
                                     HdPhInstancer const *instancer = nullptr)
{
  TfTokenVector filterNames = prim->GetBuiltinPrimvarNames();

  HdPhShaderCodeSharedPtr materialShader = drawItem->GetMaterialShader();
  if (materialShader) {
    TfTokenVector const &names = materialShader->GetPrimvarNames();
    filterNames.insert(filterNames.end(), names.begin(), names.end());
  }
  if (instancer) {
    TfTokenVector const &names = instancer->GetBuiltinPrimvarNames();
    filterNames.insert(filterNames.end(), names.begin(), names.end());
  }
  return filterNames;
}

static HdPrimvarDescriptorVector _FilterPrimvarDescriptors(HdPrimvarDescriptorVector primvars,
                                                           TfTokenVector const &filterNames)
{
  primvars.erase(std::remove_if(primvars.begin(),
                                primvars.end(),
                                [&filterNames](HdPrimvarDescriptor const &desc) {
                                  return std::find(filterNames.begin(),
                                                   filterNames.end(),
                                                   desc.name) == filterNames.end();
                                }),
                 primvars.end());

  return primvars;
}

HdPrimvarDescriptorVector HdPhGetPrimvarDescriptors(HdRprim const *prim,
                                                    HdPhDrawItem const *drawItem,
                                                    HdSceneDelegate *delegate,
                                                    HdInterpolation interpolation)
{
  HdPrimvarDescriptorVector primvars = prim->GetPrimvarDescriptors(delegate, interpolation);

  if (_IsEnabledPrimvarFiltering(drawItem)) {
    TfTokenVector filterNames = _GetFilterNames(prim, drawItem);

    return _FilterPrimvarDescriptors(primvars, filterNames);
  }

  return primvars;
}

HdPrimvarDescriptorVector HdPhGetInstancerPrimvarDescriptors(HdPhInstancer const *instancer,
                                                             HdSceneDelegate *delegate)
{
  HdPrimvarDescriptorVector primvars = delegate->GetPrimvarDescriptors(instancer->GetId(),
                                                                       HdInterpolationInstance);

  // XXX: Can we do filtering?
  return primvars;
}

// -----------------------------------------------------------------------------
// Material shader utility
// -----------------------------------------------------------------------------
HDPH_API
HdPhShaderCodeSharedPtr HdPhGetMaterialShader(HdRprim const *prim,
                                              HdSceneDelegate *delegate,
                                              std::string const &mixinSource)
{
  SdfPath const &materialId = prim->GetMaterialId();

  // Resolve the prim's material or use the fallback material.
  HdRenderIndex &renderIndex   = delegate->GetRenderIndex();
  HdPhMaterial const *material = static_cast<HdPhMaterial const *>(
      renderIndex.GetSprim(HdPrimTypeTokens->material, materialId));
  if (material == nullptr) {
    TF_DEBUG(HD_RPRIM_UPDATED).Msg("Using fallback material for %s\n", prim->GetId().GetText());

    material = static_cast<HdPhMaterial const *>(
        renderIndex.GetFallbackSprim(HdPrimTypeTokens->material));
  }

  // Augment the shader source if mixinSource is provided.
  HdPhShaderCodeSharedPtr shaderCode = material->GetShaderCode();
  if (!mixinSource.empty()) {
    shaderCode.reset(new HdPhMixinShader(mixinSource, shaderCode));
  }

  return shaderCode;
}

// -----------------------------------------------------------------------------
// Primvar processing and BAR allocation utilities
// -----------------------------------------------------------------------------
bool HdPhIsValidBAR(HdBufferArrayRangeSharedPtr const &range)
{
  return (range && range->IsValid());
}

bool HdPhCanSkipBARAllocationOrUpdate(HdBufferSourceSharedPtrVector const &sources,
                                      HdPhComputationSharedPtrVector const &computations,
                                      HdBufferArrayRangeSharedPtr const &curRange,
                                      HdDirtyBits dirtyBits)
{
  TF_UNUSED(dirtyBits);
  // XXX: DirtyPrimvar is serving a double role of indicating primvar value
  // dirtyness as well as descriptor dirtyness.
  // We should move to a separate dirty bit for the latter.
  bool mayHaveDirtyPrimvarDesc = (dirtyBits & HdChangeTracker::DirtyPrimvar);

  // If we have no buffer/computation sources, we can skip processing in the
  // following cases:
  // - we haven't allocated a BAR previously
  // - we have an existing BAR and its primvar descriptors haven't changed
  bool noDataSourcesToUpdate = sources.empty() && computations.empty();
  return noDataSourcesToUpdate && (!HdPhIsValidBAR(curRange) || !mayHaveDirtyPrimvarDesc);
}

bool HdPhCanSkipBARAllocationOrUpdate(HdBufferSourceSharedPtrVector const &sources,
                                      HdBufferArrayRangeSharedPtr const &curRange,
                                      HdDirtyBits dirtyBits)
{
  return HdPhCanSkipBARAllocationOrUpdate(
      sources, HdPhComputationSharedPtrVector(), curRange, dirtyBits);
}

HdBufferSpecVector HdPhGetRemovedPrimvarBufferSpecs(
    HdBufferArrayRangeSharedPtr const &curRange,
    HdPrimvarDescriptorVector const &newPrimvarDescs,
    HdExtComputationPrimvarDescriptorVector const &newCompPrimvarDescs,
    TfTokenVector const &internallyGeneratedPrimvarNames,
    SdfPath const &rprimId)
{
  if (!HdPhIsValidBAR(curRange)) {
    return HdBufferSpecVector();
  }

  HdBufferSpecVector removedPrimvarSpecs;
  // Get the new list of primvar sources for the BAR. We need to use both
  // the primvar descriptor list (that we get via the scene delegate), as
  // well as any internally generated primvars that are always added (such as
  // primId). This may contain primvars that fail validation, but we're only
  // interested in finding out existing primvars that aren't in the list.
  TfTokenVector newPrimvarNames;
  newPrimvarNames.reserve(newPrimvarDescs.size());
  for (auto const &desc : newPrimvarDescs) {
    newPrimvarNames.emplace_back(desc.name);
  }
  for (auto const &desc : newCompPrimvarDescs) {
    newPrimvarNames.emplace_back(desc.name);
  }

  // Get the buffer specs for the existing BAR...
  HdBufferSpecVector curBarSpecs;
  curRange->GetBufferSpecs(&curBarSpecs);

  // ... and check if it has buffers that are neither in the new source list
  // nor are internally generated.
  for (auto const &spec : curBarSpecs) {

    bool isInNewList = std::find(newPrimvarNames.begin(), newPrimvarNames.end(), spec.name) !=
                       newPrimvarNames.end();

    if (isInNewList) {
      continue;  // avoid the search below
    }

    bool isInGeneratedList = std::find(internallyGeneratedPrimvarNames.begin(),
                                       internallyGeneratedPrimvarNames.end(),
                                       spec.name) != internallyGeneratedPrimvarNames.end();

    if (!isInGeneratedList) {
      TF_DEBUG(HD_RPRIM_UPDATED)
          .Msg("%s: Found primvar %s that has been removed\n",
               rprimId.GetText(),
               spec.name.GetText());
      removedPrimvarSpecs.emplace_back(spec);
    }
  }

  return removedPrimvarSpecs;
}

HdBufferSpecVector HdPhGetRemovedPrimvarBufferSpecs(
    HdBufferArrayRangeSharedPtr const &curRange,
    HdPrimvarDescriptorVector const &newPrimvarDescs,
    TfTokenVector const &internallyGeneratedPrimvarNames,
    SdfPath const &rprimId)
{
  return HdPhGetRemovedPrimvarBufferSpecs(curRange,
                                          newPrimvarDescs,
                                          HdExtComputationPrimvarDescriptorVector(),
                                          internallyGeneratedPrimvarNames,
                                          rprimId);
}

void HdPhUpdateDrawItemBAR(HdBufferArrayRangeSharedPtr const &newRange,
                           int drawCoordIndex,
                           HdRprimSharedData *sharedData,
                           HdRenderIndex &renderIndex)
{
  if (!sharedData) {
    TF_CODING_ERROR("Null shared data ptr received\n");
    return;
  }

  HdBufferArrayRangeSharedPtr const &curRange = sharedData->barContainer.Get(drawCoordIndex);
  SdfPath const &id                           = sharedData->rprimID;

  if (curRange == newRange) {
    // Nothing to do. The draw item's BAR hasn't been changed.
    TF_DEBUG(HD_RPRIM_UPDATED)
        .Msg("%s: BAR at draw coord %d is still (%p)\n",
             id.GetText(),
             drawCoordIndex,
             curRange.get());

    return;
  }

  bool const curRangeValid = HdPhIsValidBAR(curRange);
  bool const newRangeValid = HdPhIsValidBAR(newRange);

  if (curRangeValid) {
    renderIndex.GetChangeTracker().SetGarbageCollectionNeeded();

    TF_DEBUG(HD_RPRIM_UPDATED)
        .Msg(
            "%s: Marking garbage collection needed to possibly reclaim BAR %p"
            " at draw coord index %d\n",
            id.GetText(),
            (void *)curRange.get(),
            drawCoordIndex);
  }

  // Flag deep batch invalidation for the following scenarios:
  // 1. Invalid <-> Valid transitions.
  // 2. When the new range is associated with a buffer array that
  // fails the aggregation test (used during batching).
  // 3. When the dispatch buffer needs to be updated for MDI batches.
  //    Note: This is needed only for indirect draw batches to update the
  //    dispatch buffer, but we prefer to not hardcode a check for
  //    the same.
  bool const rebuildDispatchBuffer = curRangeValid && newRangeValid &&
                                     curRange->GetElementOffset() != newRange->GetElementOffset();

  if (curRangeValid != newRangeValid || !newRange->IsAggregatedWith(curRange) ||
      rebuildDispatchBuffer) {

    renderIndex.GetChangeTracker().MarkBatchesDirty();

    if (TfDebug::IsEnabled(HD_RPRIM_UPDATED)) {
      if (curRangeValid != newRangeValid) {
        TfDebug::Helper().Msg(
            "%s: Marking all batches dirty due to an invalid <-> valid"
            " transition (new BAR %p, existing BAR %p)\n",
            id.GetText(),
            newRange.get(),
            curRange.get());
      }
      else if (!newRange->IsAggregatedWith(curRange)) {
        TfDebug::Helper().Msg(
            "%s: Marking all batches dirty since the new BAR (%p) "
            "doesn't aggregate with the existing BAR (%p)\n",
            id.GetText(),
            newRange.get(),
            curRange.get());
      }
      else {
        TfDebug::Helper().Msg(
            "%s: Marking all batches dirty since the new BAR (%p) "
            "doesn't aggregate with the existing BAR (%p)\n",
            id.GetText(),
            newRange.get(),
            curRange.get());
      }
    }
  }

  if (TfDebug::IsEnabled(HD_RPRIM_UPDATED)) {
    TfDebug::Helper().Msg("%s: Updating BAR at draw coord index %d from %p to %p\n",
                          id.GetText(),
                          drawCoordIndex,
                          curRange.get(),
                          newRange.get());

    if (newRangeValid) {
      TfDebug::Helper().Msg("Buffer array version for the new range is %lu\n",
                            newRange->GetVersion());
    }

    HdBufferSpecVector oldSpecs;
    if (curRangeValid) {
      curRange->GetBufferSpecs(&oldSpecs);
    }
    HdBufferSpecVector newSpecs;
    if (newRangeValid) {
      newRange->GetBufferSpecs(&newSpecs);
    }
    if (oldSpecs != newSpecs) {
      TfDebug::Helper().Msg("Old buffer specs:\n");
      HdBufferSpec::Dump(oldSpecs);

      TfDebug::Helper().Msg("New buffer specs:\n");
      HdBufferSpec::Dump(newSpecs);
    }
  }

  // Note: This should happen at the end since curRange is a reference to
  // the BAR at the drawCoordIndex.
  sharedData->barContainer.Set(drawCoordIndex, newRange);
}

bool HdPhIsPrimvarExistentAndValid(HdRprim *prim,
                                   HdSceneDelegate *delegate,
                                   HdPrimvarDescriptorVector const &primvars,
                                   TfToken const &primvarName)
{
  SdfPath const &id = prim->GetId();

  for (const HdPrimvarDescriptor &pv : primvars) {
    // Note: the value check here should match
    // HdPhIsInstancePrimvarExistentAndValid.
    if (pv.name == primvarName) {
      VtValue value = delegate->Get(id, pv.name);

      if (value.IsHolding<std::string>() || value.IsHolding<VtStringArray>()) {
        return false;
      }

      if (value.IsArrayValued() && value.GetArraySize() == 0) {
        // Catch empty arrays
        return false;
      }

      return (!value.IsEmpty());
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
// Constant primvar processing utilities
// -----------------------------------------------------------------------------
bool HdPhShouldPopulateConstantPrimvars(HdDirtyBits const *dirtyBits, SdfPath const &id)
{
  return HdChangeTracker::IsAnyPrimvarDirty(*dirtyBits, id) ||
         HdChangeTracker::IsTransformDirty(*dirtyBits, id) ||
         HdChangeTracker::IsExtentDirty(*dirtyBits, id) ||
         HdChangeTracker::IsPrimIdDirty(*dirtyBits, id);
}

void HdPhPopulateConstantPrimvars(HdRprim *prim,
                                  HdRprimSharedData *sharedData,
                                  HdSceneDelegate *delegate,
                                  HdDrawItem *drawItem,
                                  HdDirtyBits *dirtyBits,
                                  HdPrimvarDescriptorVector const &constantPrimvars,
                                  bool *hasMirroredTransform)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  SdfPath const &id          = prim->GetId();
  SdfPath const &instancerId = prim->GetInstancerId();

  HdRenderIndex &renderIndex = delegate->GetRenderIndex();
  HdPhResourceRegistrySharedPtr const &hdPhResourceRegistry =
      std::static_pointer_cast<HdPhResourceRegistry>(renderIndex.GetResourceRegistry());

  // Update uniforms
  HdBufferSourceSharedPtrVector sources;
  if (HdChangeTracker::IsTransformDirty(*dirtyBits, id)) {
    GfMatrix4d transform = delegate->GetTransform(id);
    sharedData->bounds.SetMatrix(transform);  // for CPU frustum culling

    HdBufferSourceSharedPtr source(new HdVtBufferSource(HdTokens->transform, transform));
    sources.push_back(source);
    source.reset(new HdVtBufferSource(HdTokens->transformInverse, transform.GetInverse()));
    sources.push_back(source);

    bool leftHanded = transform.IsLeftHanded();

    // If this is a prototype (has instancer),
    // also push the instancer transform separately.
    if (!instancerId.IsEmpty()) {
      // Gather all instancer transforms in the instancing hierarchy
      VtMatrix4dArray rootTransforms = prim->GetInstancerTransforms(delegate);
      VtMatrix4dArray rootInverseTransforms(rootTransforms.size());
      for (size_t i = 0; i < rootTransforms.size(); ++i) {
        rootInverseTransforms[i] = rootTransforms[i].GetInverse();
        // Flip the handedness if necessary
        leftHanded ^= rootTransforms[i].IsLeftHanded();
      }

      source.reset(new HdVtBufferSource(
          HdInstancerTokens->instancerTransform, rootTransforms, rootTransforms.size()));
      sources.push_back(source);
      source.reset(new HdVtBufferSource(HdInstancerTokens->instancerTransformInverse,
                                        rootInverseTransforms,
                                        rootInverseTransforms.size()));
      sources.push_back(source);

      // XXX: It might be worth to consider to have isFlipped
      // for non-instanced prims as well. It can improve
      // the drawing performance on older-GPUs by reducing
      // fragment shader cost, although it needs more GPU memory.

      // Set as int (GLSL needs 32-bit align for bool)
      source.reset(new HdVtBufferSource(HdTokens->isFlipped, VtValue(int(leftHanded))));
      sources.push_back(source);
    }

    if (hasMirroredTransform) {
      *hasMirroredTransform = leftHanded;
    }
  }
  if (HdChangeTracker::IsExtentDirty(*dirtyBits, id)) {
    // Note: If the scene description doesn't provide the extents, we use
    // the default constructed GfRange3d which is [FLT_MAX, -FLT_MAX],
    // which disables frustum culling for the prim.
    sharedData->bounds.SetRange(prim->GetExtent(delegate));

    GfVec3d const &localMin = drawItem->GetBounds().GetBox().GetMin();
    HdBufferSourceSharedPtr sourceMin(new HdVtBufferSource(
        HdTokens->bboxLocalMin, VtValue(GfVec4f(localMin[0], localMin[1], localMin[2], 1.0f))));
    sources.push_back(sourceMin);

    GfVec3d const &localMax = drawItem->GetBounds().GetBox().GetMax();
    HdBufferSourceSharedPtr sourceMax(new HdVtBufferSource(
        HdTokens->bboxLocalMax, VtValue(GfVec4f(localMax[0], localMax[1], localMax[2], 1.0f))));
    sources.push_back(sourceMax);
  }

  if (HdChangeTracker::IsPrimIdDirty(*dirtyBits, id)) {
    int32_t primId = prim->GetPrimId();
    HdBufferSourceSharedPtr source(new HdVtBufferSource(HdTokens->primID, VtValue(primId)));
    sources.push_back(source);
  }

  if (HdChangeTracker::IsAnyPrimvarDirty(*dirtyBits, id)) {
    sources.reserve(sources.size() + constantPrimvars.size());
    for (const HdPrimvarDescriptor &pv : constantPrimvars) {
      if (HdChangeTracker::IsPrimvarDirty(*dirtyBits, id, pv.name)) {
        VtValue value = delegate->Get(id, pv.name);

        // XXX Phoenix doesn't support string primvars yet
        if (value.IsHolding<std::string>() || value.IsHolding<VtStringArray>()) {
          continue;
        }

        if (value.IsArrayValued() && value.GetArraySize() == 0) {
          // A value holding an empty array does not count as an
          // empty value. Catch that case here.
          //
          // Do nothing in this case.
        }
        else if (!value.IsEmpty()) {
          // Given that this is a constant primvar, if it is
          // holding VtArray then use that as a single array
          // value rather than as one value per element.
          HdBufferSourceSharedPtr source(new HdVtBufferSource(
              pv.name, value, value.IsArrayValued() ? value.GetArraySize() : 1));

          TF_VERIFY(source->GetTupleType().type != HdTypeInvalid);
          TF_VERIFY(source->GetTupleType().count > 0);
          sources.push_back(source);
        }
      }
    }
  }

  HdBufferArrayRangeSharedPtr const &bar = drawItem->GetConstantPrimvarRange();

  if (HdPhCanSkipBARAllocationOrUpdate(sources, bar, *dirtyBits)) {
    return;
  }

  // XXX: This should be based off the DirtyPrimvarDesc bit.
  bool hasDirtyPrimvarDesc = (*dirtyBits & HdChangeTracker::DirtyPrimvar);
  HdBufferSpecVector removedSpecs;
  if (hasDirtyPrimvarDesc) {
    static TfTokenVector internallyGeneratedPrimvars = {
        HdTokens->transform,
        HdTokens->transformInverse,
        HdInstancerTokens->instancerTransform,
        HdInstancerTokens->instancerTransformInverse,
        HdTokens->isFlipped,
        HdTokens->bboxLocalMin,
        HdTokens->bboxLocalMax,
        HdTokens->primID};
    removedSpecs = HdPhGetRemovedPrimvarBufferSpecs(
        bar, constantPrimvars, internallyGeneratedPrimvars, id);
  }

  HdBufferSpecVector bufferSpecs;
  HdBufferSpec::GetBufferSpecs(sources, &bufferSpecs);

  HdBufferArrayRangeSharedPtr range = hdPhResourceRegistry->UpdateShaderStorageBufferArrayRange(
      HdTokens->primvar, bar, bufferSpecs, removedSpecs, HdBufferArrayUsageHint());

  HdPhUpdateDrawItemBAR(
      range, drawItem->GetDrawingCoord()->GetConstantPrimvarIndex(), sharedData, renderIndex);

  TF_VERIFY(drawItem->GetConstantPrimvarRange()->IsValid());

  if (!sources.empty()) {
    hdPhResourceRegistry->AddSources(drawItem->GetConstantPrimvarRange(), std::move(sources));
  }
}

// -----------------------------------------------------------------------------
// Instancer processing utilities
// -----------------------------------------------------------------------------

void HdPhUpdateInstancerData(HdRenderIndex &renderIndex,
                             HdRprim *prim,
                             HdPhDrawItem *drawItem,
                             HdRprimSharedData *sharedData,
                             HdDirtyBits rprimDirtyBits)
{
  // If there's nothing to do, bail.
  if (!(rprimDirtyBits & HdChangeTracker::DirtyInstancer)) {
    return;
  }

  // XXX: This belongs in HdRenderIndex!!!
  HdInstancer::_SyncInstancerAndParents(renderIndex, prim->GetInstancerId());

  HdDrawingCoord *drawingCoord = drawItem->GetDrawingCoord();

  // If the instance topology changes, we want to force an instance index
  // rebuild even if the index dirty bit isn't set...
  bool forceIndexRebuild = false;

  if (rprimDirtyBits & HdChangeTracker::DirtyInstancer) {
    // If the instancer topology has changed, we might need to change
    // how many levels we allocate in the drawing coord.
    int instancerLevels = HdInstancer::GetInstancerNumLevels(renderIndex, *prim);

    if (instancerLevels != sharedData->instancerLevels) {
      sharedData->barContainer.Resize(drawingCoord->GetInstancePrimvarIndex(0) + instancerLevels);
      sharedData->instancerLevels = instancerLevels;

      renderIndex.GetChangeTracker().SetGarbageCollectionNeeded();
      renderIndex.GetChangeTracker().MarkBatchesDirty();
      forceIndexRebuild = true;
    }
  }

  /* INSTANCE PRIMVARS */
  // Populate all instance primvars by backtracing hierarachy.
  int level        = 0;
  SdfPath parentId = prim->GetInstancerId();
  while (!parentId.IsEmpty()) {
    HdInstancer *instancer = renderIndex.GetInstancer(parentId);
    if (!TF_VERIFY(instancer)) {
      return;
    }
    int drawCoordIndex = drawingCoord->GetInstancePrimvarIndex(level);
    HdBufferArrayRangeSharedPtr instancerRange =
        static_cast<HdPhInstancer *>(instancer)->GetInstancePrimvarRange();

    // If we need to update the BAR, that indicates an instancing topology
    // change and we want to force an index rebuild.
    if (instancerRange != sharedData->barContainer.Get(drawCoordIndex)) {
      forceIndexRebuild = true;
    }

    // update instance primvar slot in the drawing coordinate.
    HdPhUpdateDrawItemBAR(static_cast<HdPhInstancer *>(instancer)->GetInstancePrimvarRange(),
                          drawCoordIndex,
                          sharedData,
                          renderIndex);

    parentId = instancer->GetParentId();
    ++level;
  }

  /* INSTANCE INDICES */
  // Note, GetInstanceIndices will check index sizes against primvar sizes.
  // The instance indices are a cartesian product of each level, so they need
  // to be recomputed per-rprim.
  if (HdChangeTracker::IsInstanceIndexDirty(rprimDirtyBits, prim->GetId()) || forceIndexRebuild) {
    parentId = prim->GetInstancerId();
    if (!parentId.IsEmpty()) {
      HdInstancer *instancer = renderIndex.GetInstancer(parentId);
      if (!TF_VERIFY(instancer)) {
        return;
      }

      // update instance indices
      VtIntArray instanceIndices = static_cast<HdPhInstancer *>(instancer)->GetInstanceIndices(
          prim->GetId());

      HdPhResourceRegistrySharedPtr const &resourceRegistry =
          std::static_pointer_cast<HdPhResourceRegistry>(renderIndex.GetResourceRegistry());

      // Create the bar if needed.
      if (!drawItem->GetInstanceIndexRange()) {

        // Note: we add the instance indices twice, so that frustum
        // culling can compute culledInstanceIndices as instanceIndices
        // masked by visibility.
        HdBufferSpecVector bufferSpecs;
        bufferSpecs.emplace_back(HdInstancerTokens->instanceIndices, HdTupleType{HdTypeInt32, 1});
        bufferSpecs.emplace_back(HdInstancerTokens->culledInstanceIndices,
                                 HdTupleType{HdTypeInt32, 1});

        HdBufferArrayRangeSharedPtr range = resourceRegistry->AllocateNonUniformBufferArrayRange(
            HdTokens->topology, bufferSpecs, HdBufferArrayUsageHint());

        HdPhUpdateDrawItemBAR(
            range, drawingCoord->GetInstanceIndexIndex(), sharedData, renderIndex);

        TF_VERIFY(drawItem->GetInstanceIndexRange()->IsValid());
      }

      // If the instance index range is too big to upload, it's very
      // dangerous since the shader could index into bad memory. If we're
      // not failing on asserts, we need to zero out the index array so no
      // instances draw.
      if (!TF_VERIFY(instanceIndices.size() <=
                     drawItem->GetInstanceIndexRange()->GetMaxNumElements())) {
        instanceIndices = VtIntArray();
      }

      HdBufferSourceSharedPtrVector sources;
      HdBufferSourceSharedPtr source(
          new HdVtBufferSource(HdInstancerTokens->instanceIndices, VtValue(instanceIndices)));
      sources.push_back(source);
      source.reset(new HdVtBufferSource(HdInstancerTokens->culledInstanceIndices,
                                        VtValue(instanceIndices)));
      sources.push_back(source);

      resourceRegistry->AddSources(drawItem->GetInstanceIndexRange(), std::move(sources));
    }
  }
}

bool HdPhIsInstancePrimvarExistentAndValid(HdRenderIndex &renderIndex,
                                           HdRprim *rprim,
                                           TfToken const &primvarName)
{
  SdfPath parentId = rprim->GetInstancerId();
  while (!parentId.IsEmpty()) {
    HdInstancer *instancer = renderIndex.GetInstancer(parentId);
    if (!TF_VERIFY(instancer)) {
      return false;
    }

    HdPrimvarDescriptorVector primvars = instancer->GetDelegate()->GetPrimvarDescriptors(
        instancer->GetId(), HdInterpolationInstance);

    for (const HdPrimvarDescriptor &pv : primvars) {
      // We're looking for a primvar with the given name at any level
      // (since instance primvars aggregate).  Note: the value check here
      // must match HdPhIsPrimvarExistentAndValid.
      if (pv.name == primvarName) {
        VtValue value = instancer->GetDelegate()->Get(instancer->GetId(), pv.name);
        if (value.IsHolding<std::string>() || value.IsHolding<VtStringArray>()) {
          return false;
        }
        if (value.IsArrayValued() && value.GetArraySize() == 0) {
          return false;
        }
        return (!value.IsEmpty());
      }
    }

    parentId = instancer->GetParentId();
  }

  return false;
}

// -----------------------------------------------------------------------------
// Topological invisibility utility
// -----------------------------------------------------------------------------

// Construct and return a buffer source representing visibility of the
// topological entity (e.g., face, curve, point) using one bit for the
// visibility of each indexed entity.
static HdBufferSourceSharedPtr _GetBitmaskEncodedVisibilityBuffer(VtIntArray invisibleIndices,
                                                                  int numTotalIndices,
                                                                  TfToken const &bufferName,
                                                                  SdfPath const &rprimId)
{
  size_t numBitsPerUInt = std::numeric_limits<uint32_t>::digits;  // i.e, 32
  size_t numUIntsNeeded = ceil(numTotalIndices / (float)numBitsPerUInt);
  // Initialize all bits to 1 (visible)
  VtArray<uint32_t> visibility(numUIntsNeeded, std::numeric_limits<uint32_t>::max());

  for (VtIntArray::const_iterator i = invisibleIndices.begin(), end = invisibleIndices.end();
       i != end;
       ++i) {
    if (*i >= numTotalIndices || *i < 0) {
      HF_VALIDATION_WARN(rprimId,
                         "Topological invisibility data (%d) is not in the range [0, %d)"
                         ".",
                         *i,
                         numTotalIndices);
      continue;
    }
    size_t arrayIndex = *i / numBitsPerUInt;
    size_t bitIndex   = *i % numBitsPerUInt;
    visibility[arrayIndex] &= ~(1 << bitIndex);  // set bit to 0
  }

  return HdBufferSourceSharedPtr(
      new HdVtBufferSource(bufferName, VtValue(visibility), numUIntsNeeded));
}

void HdPhProcessTopologyVisibility(VtIntArray invisibleElements,
                                   int numTotalElements,
                                   VtIntArray invisiblePoints,
                                   int numTotalPoints,
                                   HdRprimSharedData *sharedData,
                                   HdPhDrawItem *drawItem,
                                   HdChangeTracker *changeTracker,
                                   HdPhResourceRegistrySharedPtr const &resourceRegistry,
                                   SdfPath const &rprimId)
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();
  HdBufferArrayRangeSharedPtr tvBAR = drawItem->GetTopologyVisibilityRange();
  HdBufferSourceSharedPtrVector sources;

  // For the general case wherein there is no topological invisibility, we
  // don't create a BAR.
  // If any topological invisibility is authored (points/elements), create the
  // BAR with both sources. Once the BAR is created, we don't attempt to
  // delete it when there's no topological invisibility authored; we simply
  // reset the bits to make all elements/points visible.
  if (tvBAR || (!invisibleElements.empty() || !invisiblePoints.empty())) {
    sources.push_back(_GetBitmaskEncodedVisibilityBuffer(
        invisibleElements, numTotalElements, HdTokens->elementsVisibility, rprimId));
    sources.push_back(_GetBitmaskEncodedVisibilityBuffer(
        invisiblePoints, numTotalPoints, HdTokens->pointsVisibility, rprimId));
  }

  // Exit early if the BAR doesn't need to be allocated.
  if (!tvBAR && sources.empty())
    return;

  HdBufferSpecVector bufferSpecs;
  HdBufferSpec::GetBufferSpecs(sources, &bufferSpecs);
  bool barNeedsReallocation = false;
  if (tvBAR) {
    HdBufferSpecVector oldBufferSpecs;
    tvBAR->GetBufferSpecs(&oldBufferSpecs);
    if (oldBufferSpecs != bufferSpecs) {
      barNeedsReallocation = true;
    }
  }

  // XXX: Transition this code to use the Update* method instead.
  if (!tvBAR || barNeedsReallocation) {
    HdBufferArrayRangeSharedPtr range = resourceRegistry->AllocateShaderStorageBufferArrayRange(
        HdTokens->topologyVisibility, bufferSpecs, HdBufferArrayUsageHint());
    sharedData->barContainer.Set(drawItem->GetDrawingCoord()->GetTopologyVisibilityIndex(), range);

    changeTracker->MarkBatchesDirty();

    if (barNeedsReallocation) {
      changeTracker->SetGarbageCollectionNeeded();
    }
  }

  TF_VERIFY(drawItem->GetTopologyVisibilityRange()->IsValid());

  resourceRegistry->AddSources(drawItem->GetTopologyVisibilityRange(), std::move(sources));
}

bool HdPhIsEnabledSharedVertexPrimvar()
{
  static bool enabled = (TfGetEnvSetting(HDPH_ENABLE_SHARED_VERTEX_PRIMVAR) == 1);
  return enabled;
}

uint64_t HdPhComputeSharedPrimvarId(uint64_t baseId,
                                    HdBufferSourceSharedPtrVector const &sources,
                                    HdPhComputationSharedPtrVector const &computations)
{
  size_t primvarId = baseId;
  for (HdBufferSourceSharedPtr const &bufferSource : sources) {
    size_t sourceId = bufferSource->ComputeHash();
    primvarId       = ArchHash64((const char *)&sourceId, sizeof(sourceId), primvarId);

    if (bufferSource->HasPreChainedBuffer()) {
      HdBufferSourceSharedPtr src = bufferSource->GetPreChainedBuffer();

      while (src) {
        size_t chainedSourceId = bufferSource->ComputeHash();
        primvarId = ArchHash64((const char *)&chainedSourceId, sizeof(chainedSourceId), primvarId);

        src = src->GetPreChainedBuffer();
      }
    }
  }

  HdBufferSpecVector bufferSpecs;
  HdPhGetBufferSpecsFromCompuations(computations, &bufferSpecs);

  return TfHash::Combine(primvarId, bufferSpecs);
}

void HdPhGetBufferSpecsFromCompuations(HdPhComputationSharedPtrVector const &computations,
                                       HdBufferSpecVector *bufferSpecs)
{
  for (auto const &compQueuePair : computations) {
    HdComputationSharedPtr const &comp = compQueuePair.first;
    if (comp->IsValid()) {
      comp->GetBufferSpecs(bufferSpecs);
    }
  }
}

WABI_NAMESPACE_END
