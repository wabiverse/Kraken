//
// Copyright 2020 Pixar
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

#include "wabi/imaging/hdPh/textureObjectRegistry.h"

#include "wabi/imaging/hdPh/dynamicUvTextureObject.h"
#include "wabi/imaging/hdPh/ptexTextureObject.h"
#include "wabi/imaging/hdPh/subtextureIdentifier.h"
#include "wabi/imaging/hdPh/textureIdentifier.h"
#include "wabi/imaging/hdPh/textureObject.h"
#include "wabi/imaging/hdPh/udimTextureObject.h"
#include "wabi/imaging/hf/perfLog.h"

#include "wabi/base/work/loops.h"

WABI_NAMESPACE_BEGIN

HdPh_TextureObjectRegistry::HdPh_TextureObjectRegistry(HdPhResourceRegistry *registry)
  : _totalTextureMemory(0),
    _resourceRegistry(registry)
{}

HdPh_TextureObjectRegistry::~HdPh_TextureObjectRegistry() = default;

bool static _IsDynamic(const HdPhTextureIdentifier &textureId)
{
  return dynamic_cast<const HdPhDynamicUvSubtextureIdentifier *>(textureId.GetSubtextureIdentifier());
}

HdPhTextureObjectSharedPtr HdPh_TextureObjectRegistry::_MakeTextureObject(
  const HdPhTextureIdentifier &textureId,
  const HdTextureType textureType)
{
  switch (textureType)
  {
    case HdTextureType::Uv:
      if (_IsDynamic(textureId))
      {
        return std::make_shared<HdPhDynamicUvTextureObject>(textureId, this);
      }
      else
      {
        return std::make_shared<HdPhAssetUvTextureObject>(textureId, this);
      }
    case HdTextureType::Field:
      return std::make_shared<HdPhFieldTextureObject>(textureId, this);
    case HdTextureType::Ptex:
      return std::make_shared<HdPhPtexTextureObject>(textureId, this);
    case HdTextureType::Udim:
      return std::make_shared<HdPhUdimTextureObject>(textureId, this);
  }

  TF_CODING_ERROR("Texture type not supported by texture object registry.");
  return nullptr;
}

HdPhTextureObjectSharedPtr HdPh_TextureObjectRegistry::AllocateTextureObject(
  const HdPhTextureIdentifier &textureId,
  const HdTextureType textureType)
{
  // Check with instance registry and allocate texture and sampler object
  // if first object.
  HdInstance<HdPhTextureObjectSharedPtr> inst = _textureObjectRegistry.GetInstance(TfHash()(textureId));

  if (inst.IsFirstInstance())
  {
    HdPhTextureObjectSharedPtr const texture = _MakeTextureObject(textureId, textureType);

    inst.SetValue(texture);
    _dirtyTextures.push_back(texture);
    // Note that this is already protected by the lock that inst
    // holds for the _textureObjectRegistry.
    _filePathToTextureObjects[textureId.GetFilePath()].push_back(texture);
  }

  return inst.GetValue();
}

void HdPh_TextureObjectRegistry::MarkTextureFilePathDirty(const TfToken &filePath)
{
  _dirtyFilePaths.push_back(filePath);
}

void HdPh_TextureObjectRegistry::MarkTextureObjectDirty(HdPhTextureObjectPtr const &texture)
{
  _dirtyTextures.push_back(texture);
}

void HdPh_TextureObjectRegistry::AdjustTotalTextureMemory(const int64_t memDiff)
{
  _totalTextureMemory.fetch_add(memDiff);
}

// Turn a vector into a set, dropping expired weak points.
template<typename T, typename U>
static void _Uniquify(const T &objects, std::set<std::shared_ptr<U>> *result)
{
  // Creating a std:set might be expensive.
  //
  // Alternatives include an unordered set or a timestamp
  // mechanism, i.e., the registry stores an integer that gets
  // increased on each commit and each texture object stores an
  // integer which gets updated when a texture object is processed
  // during commit so that it can be checked whether a texture
  // object has been already processed when it gets encoutered for
  // the second time in the _dirtyTextures vector.

  TRACE_FUNCTION();
  for (std::weak_ptr<U> const &objectPtr : objects)
  {
    if (std::shared_ptr<U> const object = objectPtr.lock())
    {
      result->insert(object);
    }
  }
}

// Variable left from a time when Hio_StbImage was not thread-safe
// and testUsdImagingGLTextureWrapPhoenixTextureSystem produced
// wrong and non-deterministic results.
//
static const bool _isGlfBaseTextureDataThreadSafe = true;

std::set<HdPhTextureObjectSharedPtr> HdPh_TextureObjectRegistry::Commit()
{
  TRACE_FUNCTION();

  std::set<HdPhTextureObjectSharedPtr> result;

  // Record all textures as dirty corresponding to file paths
  // explicitly marked dirty by client.
  for (const TfToken &dirtyFilePath : _dirtyFilePaths)
  {
    const auto it = _filePathToTextureObjects.find(dirtyFilePath);
    if (it != _filePathToTextureObjects.end())
    {
      _Uniquify(it->second, &result);
    }
  }

  // Also record all textures explicitly marked dirty.
  _Uniquify(_dirtyTextures, &result);

  {
    TRACE_FUNCTION_SCOPE("Loading textures");
    HF_TRACE_FUNCTION_SCOPE("Loading textures");

    if (_isGlfBaseTextureDataThreadSafe)
    {
      // Loading a texture file of a previously unseen type might
      // require loading a new plugin, so give up the GIL temporarily
      // to the threads loading the images.
      TF_PY_ALLOW_THREADS_IN_SCOPE();

      // Parallel load texture files
      WorkParallelForEach(
        result.begin(), result.end(), [](const HdPhTextureObjectSharedPtr &texture) { texture->_Load(); });
    }
    else
    {
      for (const HdPhTextureObjectSharedPtr &texture : result)
      {
        texture->_Load();
      }
    }
  }

  {
    TRACE_FUNCTION_SCOPE("Commiting textures");
    HF_TRACE_FUNCTION_SCOPE("Committing textures");

    // Commit loaded files to GPU.
    for (const HdPhTextureObjectSharedPtr &texture : result)
    {
      texture->_Commit();
    }
  }

  _dirtyFilePaths.clear();
  _dirtyTextures.clear();

  return result;
}

// Remove all expired weak pointers from vector, return true
// if no weak pointers left.
static bool _GarbageCollect(HdPhTextureObjectPtrVector *const vec)
{
  // Go from left to right, filling slots that became empty
  // with valid weak pointers from the right.
  size_t last = vec->size();

  for (size_t i = 0; i < last; i++)
  {
    if ((*vec)[i].expired())
    {
      while (true)
      {
        last--;
        if (i == last)
        {
          break;
        }
        if (!(*vec)[last].expired())
        {
          (*vec)[i] = (*vec)[last];
          break;
        }
      }
    }
  }

  vec->resize(last);

  return last == 0;
}

static void _GarbageCollect(std::unordered_map<TfToken, HdPhTextureObjectPtrVector, TfToken::HashFunctor>
                              *const filePathToTextureObjects)
{
  for (auto it = filePathToTextureObjects->begin(); it != filePathToTextureObjects->end();)
  {

    if (_GarbageCollect(&it->second))
    {
      it = filePathToTextureObjects->erase(it);
    }
    else
    {
      ++it;
    }
  }
}

void HdPh_TextureObjectRegistry::GarbageCollect()
{
  TRACE_FUNCTION();

  _textureObjectRegistry.GarbageCollect();

  _GarbageCollect(&_filePathToTextureObjects);
}

WABI_NAMESPACE_END
