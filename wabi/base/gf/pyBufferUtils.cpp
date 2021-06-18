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

#include "wabi/base/gf/pyBufferUtils.h"
#include "wabi/base/gf/half.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

namespace
{

template<class T>
constexpr char PyFmtFor();
template<>
constexpr char PyFmtFor<bool>()
{
  return '?';
}
template<>
constexpr char PyFmtFor<char>()
{
  return 'b';
}
template<>
constexpr char PyFmtFor<unsigned char>()
{
  return 'B';
}
template<>
constexpr char PyFmtFor<short>()
{
  return 'h';
}
template<>
constexpr char PyFmtFor<unsigned short>()
{
  return 'H';
}
template<>
constexpr char PyFmtFor<int>()
{
  return 'i';
}
template<>
constexpr char PyFmtFor<unsigned int>()
{
  return 'I';
}
template<>
constexpr char PyFmtFor<long>()
{
  return 'l';
}
template<>
constexpr char PyFmtFor<unsigned long>()
{
  return 'L';
}
template<>
constexpr char PyFmtFor<GfHalf>()
{
  return 'e';
}
template<>
constexpr char PyFmtFor<float>()
{
  return 'f';
}
template<>
constexpr char PyFmtFor<double>()
{
  return 'd';
}

}  // namespace

template<class T>
char *Gf_GetPyBufferFmtFor()
{
  static char str[2] = {PyFmtFor<T>(), '\0'};
  return str;
}

template GF_API char *Gf_GetPyBufferFmtFor<bool>();
template GF_API char *Gf_GetPyBufferFmtFor<char>();
template GF_API char *Gf_GetPyBufferFmtFor<unsigned char>();
template GF_API char *Gf_GetPyBufferFmtFor<short>();
template GF_API char *Gf_GetPyBufferFmtFor<unsigned short>();
template GF_API char *Gf_GetPyBufferFmtFor<int>();
template GF_API char *Gf_GetPyBufferFmtFor<unsigned int>();
template GF_API char *Gf_GetPyBufferFmtFor<long>();
template GF_API char *Gf_GetPyBufferFmtFor<unsigned long>();
template GF_API char *Gf_GetPyBufferFmtFor<GfHalf>();
template GF_API char *Gf_GetPyBufferFmtFor<float>();
template GF_API char *Gf_GetPyBufferFmtFor<double>();

WABI_NAMESPACE_END
