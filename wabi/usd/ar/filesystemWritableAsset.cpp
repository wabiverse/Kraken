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

#include "wabi/wabi.h"

#include "wabi/usd/ar/filesystemWritableAsset.h"

#include "wabi/base/arch/errno.h"
#include "wabi/base/arch/fileSystem.h"
#include "wabi/base/tf/diagnostic.h"
#include "wabi/base/tf/errorMark.h"
#include "wabi/base/tf/fileUtils.h"
#include "wabi/base/tf/safeOutputFile.h"

WABI_NAMESPACE_BEGIN

std::shared_ptr<ArFilesystemWritableAsset> ArFilesystemWritableAsset::Create(
    const ArResolvedPath &resolvedPath,
    ArResolver::WriteMode writeMode)
{
  const std::string dir = TfGetPathName(resolvedPath);
  if (!dir.empty() && !TfIsDir(dir) && !TfMakeDirs(dir)) {
    TF_RUNTIME_ERROR("Could not create directory '%s' for asset '%s'",
                     dir.c_str(),
                     resolvedPath.GetPathString().c_str());
    return nullptr;
  }

  TfErrorMark m;

  TfSafeOutputFile f;
  switch (writeMode) {
    case ArResolver::WriteMode::Update:
      f = TfSafeOutputFile::Update(resolvedPath);
      break;
    case ArResolver::WriteMode::Replace:
      f = TfSafeOutputFile::Replace(resolvedPath);
      break;
  }

  if (!m.IsClean()) {
    return nullptr;
  }

  return std::make_shared<ArFilesystemWritableAsset>(std::move(f));
}

ArFilesystemWritableAsset::ArFilesystemWritableAsset(TfSafeOutputFile &&file)
    : _file(std::move(file))
{
  if (!_file.Get()) {
    TF_CODING_ERROR("Invalid output file");
  }
}

ArFilesystemWritableAsset::~ArFilesystemWritableAsset() = default;

bool ArFilesystemWritableAsset::Close()
{
  TfErrorMark m;
  _file.Close();
  return m.IsClean();
}

size_t ArFilesystemWritableAsset::Write(const void *buffer, size_t count, size_t offset)
{
  int64_t numWritten = ArchPWrite(_file.Get(), buffer, count, offset);
  if (numWritten == -1) {
    TF_RUNTIME_ERROR("Error occurred writing file: %s", ArchStrerror().c_str());
    return 0;
  }
  return numWritten;
}

WABI_NAMESPACE_END
