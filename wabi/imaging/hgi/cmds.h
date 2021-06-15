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
#ifndef WABI_IMAGING_HGI_CMDS_H
#define WABI_IMAGING_HGI_CMDS_H

#include "wabi/imaging/hgi/api.h"
#include "wabi/imaging/hgi/enums.h"
#include "wabi/wabi.h"
#include <memory>

WABI_NAMESPACE_BEGIN

class Hgi;

using HgiCmdsUniquePtr = std::unique_ptr<class HgiCmds>;

/// \class HgiCmds
///
/// Graphics commands are recorded in 'cmds' objects which are later submitted
/// to hgi. HgiCmds is the base class for other cmds objects.
///
class HgiCmds {
 public:
  HGI_API
  virtual ~HgiCmds();

  /// Returns true if the HgiCmds object has been submitted to GPU.
  HGI_API
  bool IsSubmitted() const;

 protected:
  friend class Hgi;

  HGI_API
  HgiCmds();

  // Submit can be called inside of Hgi::SubmitCmds to commit the
  // command buffer to the GPU. Returns true if work was committed.
  // The default implementation returns false.
  HGI_API
  virtual bool _Submit(Hgi *hgi, HgiSubmitWaitType wait);

  // Flags the HgiCmds object as 'submitted' to GPU.
  HGI_API
  void _SetSubmitted();

 private:
  HgiCmds &operator=(const HgiCmds &) = delete;
  HgiCmds(const HgiCmds &) = delete;

  bool _submitted;
};

WABI_NAMESPACE_END

#endif
