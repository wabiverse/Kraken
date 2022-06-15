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
#ifndef WABI_USD_PLUGIN_USD_ABC_ALEMBIC_WRITER_H
#define WABI_USD_PLUGIN_USD_ABC_ALEMBIC_WRITER_H

/// \file usdAbc/alembicWriter.h

#include "wabi/base/tf/declarePtrs.h"
#include "wabi/base/tf/staticTokens.h"
#include "wabi/wabi.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <set>
#include <string>

WABI_NAMESPACE_BEGIN

// Note -- Even though this header is private we try to keep Alembic headers
//         out of it anyway for simplicity's sake.

TF_DECLARE_WEAK_AND_REF_PTRS(SdfAbstractData);

/// \class UsdAbc_AlembicDataWriter
///
/// An alembic writer suitable for an SdfAbstractData.
///
class UsdAbc_AlembicDataWriter : boost::noncopyable
{
 public:

  UsdAbc_AlembicDataWriter();
  ~UsdAbc_AlembicDataWriter();

  bool Open(const std::string &filePath, const std::string &comment);
  bool Write(const SdfAbstractDataConstPtr &data);
  bool Close();

  bool IsValid() const;
  std::string GetErrors() const;

  void SetFlag(const TfToken &, bool set = true);

 private:

  boost::scoped_ptr<class UsdAbc_AlembicDataWriterImpl> _impl;
  std::string _errorLog;
};

WABI_NAMESPACE_END

#endif
