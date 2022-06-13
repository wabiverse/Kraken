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
#include "wabi/imaging/hdPh/unitTestHelper.h"
#include "wabi/imaging/hdPh/renderPass.h"
#include "wabi/imaging/hdPh/resourceBinder.h"

#include "wabi/imaging/hd/camera.h"
#include "wabi/imaging/hd/rprimCollection.h"
#include "wabi/imaging/hd/tokens.h"

#include "wabi/imaging/hgi/hgi.h"
#include "wabi/imaging/hgi/tokens.h"

#include "wabi/imaging/glf/diagnostic.h"

#include "wabi/base/gf/frustum.h"
#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/tf/getenv.h"
#include "wabi/base/tf/staticTokens.h"

#include <sstream>
#include <string>

WABI_NAMESPACE_BEGIN

TF_DEFINE_PRIVATE_TOKENS(_tokens,
                         (l0dir)(l0color)(l1dir)(l1color)(sceneAmbient)(vec3)

                         // Collection names
                         (testCollection));

class HdPh_DrawTask final : public HdTask
{
 public:

  HdPh_DrawTask(HdRenderPassSharedPtr const &renderPass,
                HdPhRenderPassStateSharedPtr const &renderPassState,
                bool withGuides)
    : HdTask(SdfPath::EmptyPath()),
      _renderPass(renderPass),
      _renderPassState(renderPassState),
      _renderTags()
  {
    _renderTags.reserve(2);
    _renderTags.push_back(HdRenderTagTokens->geometry);

    if (withGuides) {
      _renderTags.push_back(HdRenderTagTokens->guide);
    }
  }

  void Sync(HdSceneDelegate *, HdTaskContext *, HdDirtyBits *) override
  {
    _renderPass->Sync();
  }

  void Prepare(HdTaskContext *ctx, HdRenderIndex *renderIndex) override
  {
    _renderPassState->Prepare(renderIndex->GetResourceRegistry());
  }

  void Execute(HdTaskContext *ctx) override
  {
    _renderPass->Execute(_renderPassState, GetRenderTags());
  }

  const TfTokenVector &GetRenderTags() const override
  {
    return _renderTags;
  }

 private:

  HdRenderPassSharedPtr _renderPass;
  HdPhRenderPassStateSharedPtr _renderPassState;
  TfTokenVector _renderTags;

  HdPh_DrawTask() = delete;
  HdPh_DrawTask(const HdPh_DrawTask &) = delete;
  HdPh_DrawTask &operator=(const HdPh_DrawTask &) = delete;
};

template<typename T> static VtArray<T> _BuildArray(T values[], int numValues)
{
  VtArray<T> result(numValues);
  std::copy(values, values + numValues, result.begin());
  return result;
}

HdPh_TestDriver::HdPh_TestDriver()
  : _hgi(Hgi::CreatePlatformDefaultHgi()),
    _hgiDriver{HgiTokens->renderDriver, VtValue(_hgi.get())},
    _engine(),
    _renderDelegate(),
    _renderIndex(nullptr),
    _sceneDelegate(nullptr),
    _renderPass(),
    _renderPassState(
      std::dynamic_pointer_cast<HdPhRenderPassState>(_renderDelegate.CreateRenderPassState())),
    _collection(_tokens->testCollection, HdReprSelector())
{
  if (TfGetenv("HD_ENABLE_SMOOTH_NORMALS", "CPU") == "CPU" ||
      TfGetenv("HD_ENABLE_SMOOTH_NORMALS", "CPU") == "GPU") {
    _Init(HdReprSelector(HdReprTokens->smoothHull));
  } else {
    _Init(HdReprSelector(HdReprTokens->hull));
  }
}

HdPh_TestDriver::HdPh_TestDriver(TfToken const &reprName)
  : _hgi(Hgi::CreatePlatformDefaultHgi()),
    _hgiDriver{HgiTokens->renderDriver, VtValue(_hgi.get())},
    _engine(),
    _renderDelegate(),
    _renderIndex(nullptr),
    _sceneDelegate(nullptr),
    _renderPass(),
    _renderPassState(
      std::dynamic_pointer_cast<HdPhRenderPassState>(_renderDelegate.CreateRenderPassState())),
    _collection(_tokens->testCollection, HdReprSelector())
{
  _Init(HdReprSelector(reprName));
}

HdPh_TestDriver::HdPh_TestDriver(HdReprSelector const &reprToken)
  : _hgi(Hgi::CreatePlatformDefaultHgi()),
    _hgiDriver{HgiTokens->renderDriver, VtValue(_hgi.get())},
    _engine(),
    _renderDelegate(),
    _renderIndex(nullptr),
    _sceneDelegate(nullptr),
    _renderPass(),
    _renderPassState(
      std::dynamic_pointer_cast<HdPhRenderPassState>(_renderDelegate.CreateRenderPassState())),
    _collection(_tokens->testCollection, HdReprSelector())
{
  _Init(reprToken);
}

HdPh_TestDriver::~HdPh_TestDriver()
{
  delete _sceneDelegate;
  delete _renderIndex;
}

void HdPh_TestDriver::_Init(HdReprSelector const &reprToken)
{
  _renderIndex = HdRenderIndex::New(&_renderDelegate, {&_hgiDriver});
  TF_VERIFY(_renderIndex != nullptr);

  _sceneDelegate = new HdUnitTestDelegate(_renderIndex, SdfPath::AbsoluteRootPath());

  _cameraId = SdfPath("/testCam");
  _sceneDelegate->AddCamera(_cameraId);
  _reprToken = reprToken;

  GfMatrix4d viewMatrix = GfMatrix4d().SetIdentity();
  viewMatrix *= GfMatrix4d().SetTranslate(GfVec3d(0.0, 1000.0, 0.0));
  viewMatrix *= GfMatrix4d().SetRotate(GfRotation(GfVec3d(1.0, 0.0, 0.0), -90.0));

  GfFrustum frustum;
  frustum.SetPerspective(45, true, 1, 1.0, 10000.0);
  GfMatrix4d projMatrix = frustum.ComputeProjectionMatrix();

  SetCamera(viewMatrix, projMatrix, GfVec4d(0, 0, 512, 512));

  // set depthfunc to GL default
  _renderPassState->SetDepthFunc(HdCmpFuncLess);

  // Update collection with repr and add collection to change tracker.
  _collection.SetReprSelector(reprToken);
  HdChangeTracker &tracker = _renderIndex->GetChangeTracker();
  tracker.AddCollection(_collection.GetName());
}

void HdPh_TestDriver::Draw(bool withGuides)
{
  Draw(GetRenderPass(), withGuides);
}

void HdPh_TestDriver::Draw(HdRenderPassSharedPtr const &renderPass, bool withGuides)
{
  HdTaskSharedPtrVector tasks = {
    std::make_shared<HdPh_DrawTask>(renderPass, _renderPassState, withGuides)};
  _engine.Execute(&_sceneDelegate->GetRenderIndex(), &tasks);

  GLF_POST_PENDING_GL_ERRORS();
}

void HdPh_TestDriver::SetCamera(GfMatrix4d const &modelViewMatrix,
                                GfMatrix4d const &projectionMatrix,
                                GfVec4d const &viewport)
{
  _sceneDelegate->UpdateCamera(_cameraId,
                               HdCameraTokens->worldToViewMatrix,
                               VtValue(modelViewMatrix));
  _sceneDelegate->UpdateCamera(_cameraId,
                               HdCameraTokens->projectionMatrix,
                               VtValue(projectionMatrix));
  // Baselines for tests were generated without constraining the view
  // frustum based on the viewport aspect ratio.
  _sceneDelegate->UpdateCamera(_cameraId,
                               HdCameraTokens->windowPolicy,
                               VtValue(CameraUtilDontConform));

  HdSprim const *cam = _renderIndex->GetSprim(HdPrimTypeTokens->camera, _cameraId);
  TF_VERIFY(cam);
  _renderPassState->SetCameraAndViewport(dynamic_cast<HdCamera const *>(cam), viewport);
}

void HdPh_TestDriver::SetCameraClipPlanes(std::vector<GfVec4d> const &clipPlanes)
{
  _sceneDelegate->UpdateCamera(_cameraId, HdCameraTokens->clipPlanes, VtValue(clipPlanes));
}

void HdPh_TestDriver::SetCullStyle(HdCullStyle cullStyle)
{
  _renderPassState->SetCullStyle(cullStyle);
}

HdRenderPassSharedPtr const &HdPh_TestDriver::GetRenderPass()
{
  if (!_renderPass) {
    _renderPass = HdRenderPassSharedPtr(
      new HdPh_RenderPass(&_sceneDelegate->GetRenderIndex(), _collection));
  }
  return _renderPass;
}

void HdPh_TestDriver::SetRepr(HdReprSelector const &reprToken)
{
  _collection.SetReprSelector(reprToken);

  // Mark changes.
  HdChangeTracker &tracker = _renderIndex->GetChangeTracker();
  tracker.MarkCollectionDirty(_collection.GetName());

  // Update render pass with updated collection
  _renderPass->SetRprimCollection(_collection);
}

// --------------------------------------------------------------------------

HdPh_TestLightingShader::HdPh_TestLightingShader()
{
  const char *lightingShader =
    "-- glslfx version 0.1                                              \n"
    "-- configuration                                                   \n"
    "{\"techniques\": {\"default\": {\"fragmentShader\" : {             \n"
    " \"source\": [\"TestLighting.Lighting\"]                           \n"
    "}}}}                                                               \n"
    "-- glsl TestLighting.Lighting                                      \n"
    "vec3 FallbackLighting(vec3 Peye, vec3 Neye, vec3 color) {          \n"
    "    vec3 n = normalize(Neye);                                      \n"
    "    return HdGet_sceneAmbient()                                    \n"
    "      + color * HdGet_l0color() * max(0.0, dot(n, HdGet_l0dir()))  \n"
    "      + color * HdGet_l1color() * max(0.0, dot(n, HdGet_l1dir())); \n"
    "}                                                                  \n";

  _lights[0].dir = GfVec3f(0, 0, 1);
  _lights[0].color = GfVec3f(1, 1, 1);
  _lights[1].dir = GfVec3f(0, 0, 1);
  _lights[1].color = GfVec3f(0, 0, 0);
  _sceneAmbient = GfVec3f(0.04, 0.04, 0.04);

  std::stringstream ss(lightingShader);
  _glslfx.reset(new HioGlslfx(ss));
}

HdPh_TestLightingShader::~HdPh_TestLightingShader() {}

/* virtual */
HdPh_TestLightingShader::ID HdPh_TestLightingShader::ComputeHash() const
{
  HD_TRACE_FUNCTION();

  size_t hash = _glslfx->GetHash();
  return (ID)hash;
}

/* virtual */
std::string HdPh_TestLightingShader::GetSource(TfToken const &shaderStageKey) const
{
  HD_TRACE_FUNCTION();
  HF_MALLOC_TAG_FUNCTION();

  std::string source = _glslfx->GetSource(shaderStageKey);
  return source;
}

/* virtual */
void HdPh_TestLightingShader::SetCamera(GfMatrix4d const &worldToViewMatrix,
                                        GfMatrix4d const &projectionMatrix)
{
  for (int i = 0; i < 2; ++i) {
    _lights[i].eyeDir = worldToViewMatrix.TransformDir(_lights[i].dir).GetNormalized();
  }
}

/* virtual */
void HdPh_TestLightingShader::BindResources(const int program,
                                            HdPh_ResourceBinder const &binder,
                                            HdRenderPassState const &state)
{
  binder.BindUniformf(_tokens->l0dir, 3, _lights[0].eyeDir.GetArray());
  binder.BindUniformf(_tokens->l0color, 3, _lights[0].color.GetArray());
  binder.BindUniformf(_tokens->l1dir, 3, _lights[1].eyeDir.GetArray());
  binder.BindUniformf(_tokens->l1color, 3, _lights[1].color.GetArray());
  binder.BindUniformf(_tokens->sceneAmbient, 3, _sceneAmbient.GetArray());
}

/* virtual */
void HdPh_TestLightingShader::UnbindResources(const int program,
                                              HdPh_ResourceBinder const &binder,
                                              HdRenderPassState const &state)
{}

/*virtual*/
void HdPh_TestLightingShader::AddBindings(HdBindingRequestVector *customBindings)
{
  customBindings->emplace_back(HdBinding::UNIFORM, _tokens->l0dir, HdTypeFloatVec3);
  customBindings->emplace_back(HdBinding::UNIFORM, _tokens->l0color, HdTypeFloatVec3);
  customBindings->emplace_back(HdBinding::UNIFORM, _tokens->l1dir, HdTypeFloatVec3);
  customBindings->emplace_back(HdBinding::UNIFORM, _tokens->l1color, HdTypeFloatVec3);
  customBindings->emplace_back(HdBinding::UNIFORM, _tokens->sceneAmbient, HdTypeFloatVec3);
}

void HdPh_TestLightingShader::SetSceneAmbient(GfVec3f const &color)
{
  _sceneAmbient = color;
}

void HdPh_TestLightingShader::SetLight(int light, GfVec3f const &dir, GfVec3f const &color)
{
  if (light < 2) {
    _lights[light].dir = dir;
    _lights[light].eyeDir = dir;
    _lights[light].color = color;
  }
}

WABI_NAMESPACE_END
