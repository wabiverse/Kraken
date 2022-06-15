/************************************************************************
Copyright 2020 Advanced Micro Devices, Inc
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
************************************************************************/

#ifndef RPRUSD_UTIL_H
#define RPRUSD_UTIL_H

#include "wabi/imaging/rprUsd/api.h"

#include "wabi/imaging/garch/glApi.h"
#include "wabi/imaging/hio/image.h"

#include <string>

WABI_NAMESPACE_BEGIN

class RPRUSD_API RprUsdTextureData
{
 public:

  static std::shared_ptr<RprUsdTextureData> New(std::string const &filepath);

  uint8_t *GetData() const;
  int GetWidth() const;
  int GetHeight() const;

  struct GLMetadata
  {
    GLenum glFormat;
    GLenum glType;
    GLenum internalFormat;
  };
  GLMetadata GetGLMetadata() const;

 private:

  HioImage::StorageSpec _hioStorageSpec;
  std::unique_ptr<uint8_t[]> _data;
};

using RprUsdTextureDataRefPtr = std::shared_ptr<RprUsdTextureData>;

RPRUSD_API
bool RprUsdGetUDIMFormatString(std::string const &filepath, std::string *out_formatString);

RPRUSD_API
bool RprUsdInitGLApi();

WABI_NAMESPACE_END

#endif  // RPRUSD_UTIL_H
