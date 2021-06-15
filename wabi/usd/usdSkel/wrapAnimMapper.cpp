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
#include "wabi/usd/usdSkel/animMapper.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/matrix4f.h"

#include "wabi/base/tf/pyContainerConversions.h"
#include "wabi/base/tf/pyResultConversions.h"
#include "wabi/base/tf/pyUtils.h"
#include "wabi/base/tf/wrapTypeHelpers.h"

#include "wabi/usd/usd/pyConversions.h"

#include <boost/python.hpp>

using namespace boost::python;

WABI_NAMESPACE_USING

namespace {

TfPyObjWrapper _Remap(const UsdSkelAnimMapper &self,
                      const VtValue &source,
                      const VtValue &target,
                      int elementSize,
                      const VtValue &defaultValue)
{
  VtValue output(target);
  self.Remap(source, &output, elementSize, defaultValue);
  return UsdVtValueToPython(output);
}

template<typename Matrix4>
VtArray<Matrix4> _RemapTransforms(const UsdSkelAnimMapper &self,
                                  const VtArray<Matrix4> &source,
                                  const VtArray<Matrix4> &target,
                                  int elementSize)
{
  VtArray<Matrix4> output(target);
  self.RemapTransforms(source, &output, elementSize);
  return output;
}

}  // namespace

void wrapUsdSkelAnimMapper()
{
  using This = UsdSkelAnimMapper;

  class_<This, UsdSkelAnimMapperRefPtr>("AnimMapper", no_init)

    .def(init<>())

    .def(init<size_t>())

    .def(init<VtTokenArray, VtTokenArray>((arg("sourceOrder"), arg("targetOrder"))))
    .def("Remap",
         &_Remap,
         (arg("source"), arg("target") = VtValue(), arg("elementSize") = 1, arg("defaultValue") = VtValue()))

    .def("RemapTransforms",
         &_RemapTransforms<GfMatrix4d>,
         (arg("source"), arg("target"), arg("elementSize") = 1))

    .def("RemapTransforms",
         &_RemapTransforms<GfMatrix4f>,
         (arg("source"), arg("target"), arg("elementSize") = 1))

    .def("IsIdentity", &This::IsIdentity)

    .def("IsSparse", &This::IsSparse)

    .def("IsNull", &This::IsNull)

    .def("__len__", &This::size);
}
