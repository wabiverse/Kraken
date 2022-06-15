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
#ifndef WABI_IMAGING_HIO_IMAGE_REGISTRY_H
#define WABI_IMAGING_HIO_IMAGE_REGISTRY_H

#include "wabi/wabi.h"
#include "wabi/imaging/hio/api.h"
#include "wabi/base/tf/singleton.h"

#include <memory>
#include <string>

WABI_NAMESPACE_BEGIN

using HioImageSharedPtr = std::shared_ptr<class HioImage>;

class HioRankedTypeMap;

/// \class HioImageRegistry
///
/// Manages plugin registration and loading for HioImage subclasses.
///
class HioImageRegistry : public TfSingleton<HioImageRegistry>
{
 public:

  HIO_API
  static HioImageRegistry &GetInstance();

  HIO_API
  bool IsSupportedImageFile(std::string const &filename);

 private:

  friend class TfSingleton<HioImageRegistry>;
  HioImageRegistry();

  friend class HioImage;

  HioImageSharedPtr _ConstructImage(std::string const &filename);

 private:

  std::unique_ptr<HioRankedTypeMap> const _typeMap;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HIO_IMAGE_REGISTRY_H
