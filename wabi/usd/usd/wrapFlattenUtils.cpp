//
// Copyright 2019 Pixar
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
#include "wabi/base/arch/pragmas.h"
#include "wabi/wabi.h"

#include <boost/python/def.hpp>

#include "wabi/base/tf/makePyConstructor.h"
#include "wabi/base/tf/pyFunction.h"
#include "wabi/base/tf/pyPtrHelpers.h"
#include "wabi/base/tf/pyResultConversions.h"
#include "wabi/usd/sdf/layer.h"
#include "wabi/usd/usd/flattenUtils.h"

using namespace boost::python;

WABI_NAMESPACE_USING

static SdfLayerRefPtr _UsdFlattenLayerStack2(const PcpLayerStackRefPtr &layerStack,
                                             const std::string &tag)
{
  return UsdFlattenLayerStack(layerStack, tag);
}

using Py_UsdFlattenResolveAssetPathSig = std::string(const SdfLayerHandle &, const std::string &);
using Py_UsdFlattenResolveAssetPathFn  = std::function<Py_UsdFlattenResolveAssetPathSig>;

static SdfLayerRefPtr _UsdFlattenLayerStack3(
    const PcpLayerStackRefPtr &layerStack,
    const Py_UsdFlattenResolveAssetPathFn &resolveAssetPathFn,
    const std::string &tag)
{
  return UsdFlattenLayerStack(layerStack, resolveAssetPathFn, tag);
}

void wrapUsdFlattenUtils()
{
  def("FlattenLayerStack",
      &_UsdFlattenLayerStack2,
      (arg("layerStack"), arg("tag") = std::string()),
      boost::python::return_value_policy<TfPyRefPtrFactory<SdfLayerHandle>>());

  TfPyFunctionFromPython<Py_UsdFlattenResolveAssetPathSig>();
  def("FlattenLayerStack",
      &_UsdFlattenLayerStack3,
      (arg("layerStack"), arg("resolveAssetPathFn"), arg("tag") = std::string()),
      boost::python::return_value_policy<TfPyRefPtrFactory<SdfLayerHandle>>());

  def("FlattenLayerStackResolveAssetPath",
      UsdFlattenLayerStackResolveAssetPath,
      (arg("sourceLayer"), arg("assetPath")));
}
