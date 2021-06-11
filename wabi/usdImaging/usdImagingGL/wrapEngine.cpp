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
#include <boost/python.hpp>
#include <boost/python/class.hpp>
#include <boost/python/converter/from_python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/tuple.hpp>

#include "wabi/usdImaging/usdImagingGL/engine.h"

#include "wabi/usdImaging/usdImaging/delegate.h"

#include "wabi/base/tf/pyContainerConversions.h"
#include "wabi/base/tf/pyEnum.h"
#include "wabi/base/tf/pyResultConversions.h"
#include "wabi/usd/usd/prim.h"

using namespace std;
using namespace boost::python;
using namespace boost;

WABI_NAMESPACE_USING

namespace {

static boost::python::tuple _TestIntersection(UsdImagingGLEngine &self,
                                              const GfMatrix4d &viewMatrix,
                                              const GfMatrix4d &projectionMatrix,
                                              const UsdPrim &root,
                                              UsdImagingGLRenderParams params)
{
  GfVec3d hitPoint;
  GfVec3d hitNormal;
  SdfPath hitPrimPath;
  SdfPath hitInstancerPath;
  int hitInstanceIndex;
  HdInstancerContext hitInstancerContext;

  self.TestIntersection(viewMatrix,
                        projectionMatrix,
                        root,
                        params,
                        &hitPoint,
                        &hitNormal,
                        &hitPrimPath,
                        &hitInstancerPath,
                        &hitInstanceIndex,
                        &hitInstancerContext);

  SdfPath topLevelPath      = SdfPath::EmptyPath();
  int topLevelInstanceIndex = -1;
  if (hitInstancerContext.size() > 0) {
    topLevelPath          = hitInstancerContext[0].first;
    topLevelInstanceIndex = hitInstancerContext[0].second;
  }

  return boost::python::make_tuple(
      hitPoint, hitNormal, hitPrimPath, hitInstanceIndex, topLevelPath, topLevelInstanceIndex);
}

static void _SetLightingState(UsdImagingGLEngine &self,
                              GlfSimpleLightVector const &lights,
                              GlfSimpleMaterial const &material,
                              GfVec4f const &sceneAmbient)
{
  self.SetLightingState(lights, material, sceneAmbient);
}

void _SetOverrideWindowPolicy(UsdImagingGLEngine &self, const object &pyObj)
{
  extract<CameraUtilConformWindowPolicy> extractor(pyObj);
  if (extractor.check()) {
    self.SetOverrideWindowPolicy({true, extractor()});
  }
  else {
    self.SetOverrideWindowPolicy({false, CameraUtilFit});
  }
}

}  // anonymous namespace

void wrapEngine()
{
  {
    scope engineScope =
        class_<UsdImagingGLEngine, boost::noncopyable>("Engine", "UsdImaging Renderer class")
            .def(init<>())
            .def(init<const SdfPath &, const SdfPathVector &, const SdfPathVector &>())
            .def("Render", &UsdImagingGLEngine::Render)
            .def("SetWindowPolicy", &UsdImagingGLEngine::SetWindowPolicy)
            .def("SetRenderViewport", &UsdImagingGLEngine::SetRenderViewport)
            .def("SetCameraPath", &UsdImagingGLEngine::SetCameraPath)
            .def("SetCameraState", &UsdImagingGLEngine::SetCameraState)
            .def("SetLightingStateFromOpenGL", &UsdImagingGLEngine::SetLightingStateFromOpenGL)
            .def("SetLightingState", &_SetLightingState)
            .def("SetCameraStateFromOpenGL", &UsdImagingGLEngine::SetCameraStateFromOpenGL)
            .def("SetSelected", &UsdImagingGLEngine::SetSelected)
            .def("ClearSelected", &UsdImagingGLEngine::ClearSelected)
            .def("AddSelected", &UsdImagingGLEngine::AddSelected)
            .def("SetSelectionColor", &UsdImagingGLEngine::SetSelectionColor)
            .def("TestIntersection", &_TestIntersection)
            .def("IsHydraEnabled", &UsdImagingGLEngine::IsHydraEnabled)
            .staticmethod("IsHydraEnabled")
            .def("IsConverged", &UsdImagingGLEngine::IsConverged)
            .def("GetRendererPlugins",
                 &UsdImagingGLEngine::GetRendererPlugins,
                 return_value_policy<TfPySequenceToList>())
            .staticmethod("GetRendererPlugins")
            .def("GetRendererDisplayName", &UsdImagingGLEngine::GetRendererDisplayName)
            .staticmethod("GetRendererDisplayName")
            .def("GetCurrentRendererId", &UsdImagingGLEngine::GetCurrentRendererId)
            .def("SetRendererPlugin", &UsdImagingGLEngine::SetRendererPlugin)
            .def("GetRendererAovs",
                 &UsdImagingGLEngine::GetRendererAovs,
                 return_value_policy<TfPySequenceToList>())
            .def("SetRendererAov", &UsdImagingGLEngine::SetRendererAov)
            .def("GetRenderStats", &UsdImagingGLEngine::GetRenderStats)
            .def("GetRendererSettingsList",
                 &UsdImagingGLEngine::GetRendererSettingsList,
                 return_value_policy<TfPySequenceToList>())
            .def("GetRendererSetting", &UsdImagingGLEngine::GetRendererSetting)
            .def("SetRendererSetting", &UsdImagingGLEngine::SetRendererSetting)
            .def("SetColorCorrectionSettings", &UsdImagingGLEngine::SetColorCorrectionSettings)
            .def("IsColorCorrectionCapable", &UsdImagingGLEngine::IsColorCorrectionCapable)
            .staticmethod("IsColorCorrectionCapable")
            .def("GetRendererCommandDescriptors",
                 &UsdImagingGLEngine::GetRendererCommandDescriptors,
                 return_value_policy<TfPySequenceToList>())
            .def("InvokeRendererCommand",
                 &UsdImagingGLEngine::InvokeRendererCommand,
                 (boost::python::arg("command"), boost::python::arg("args") = HdCommandArgs()))
            .def("IsPauseRendererSupported", &UsdImagingGLEngine::IsPauseRendererSupported)
            .def("PauseRenderer", &UsdImagingGLEngine::PauseRenderer)
            .def("ResumeRenderer", &UsdImagingGLEngine::ResumeRenderer)
            .def("IsStopRendererSupported", &UsdImagingGLEngine::IsStopRendererSupported)
            .def("StopRenderer", &UsdImagingGLEngine::StopRenderer)
            .def("RestartRenderer", &UsdImagingGLEngine::RestartRenderer)
            .def("SetRenderBufferSize", &UsdImagingGLEngine::SetRenderBufferSize)
            .def("SetFraming", &UsdImagingGLEngine::SetFraming)
            .def("SetOverrideWindowPolicy", _SetOverrideWindowPolicy);
  }

  // Wrap the constants.
  scope().attr("ALL_INSTANCES") = UsdImagingDelegate::ALL_INSTANCES;

  TfPyContainerConversions::from_python_sequence<
      std::vector<GlfSimpleLight>,
      TfPyContainerConversions::variable_capacity_policy>();
}
