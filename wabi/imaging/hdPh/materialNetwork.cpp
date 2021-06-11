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

#include "wabi/imaging/hdPh/materialNetwork.h"
#include "wabi/imaging/hdPh/materialParam.h"
#include "wabi/imaging/hdPh/resourceRegistry.h"
#include "wabi/imaging/hdPh/subtextureIdentifier.h"
#include "wabi/imaging/hdPh/tokens.h"

#ifdef WITH_MATERIALX
#  include "wabi/imaging/hdPh/materialXFilter.h"
#endif

#include "wabi/imaging/hd/material.h"

#include "wabi/imaging/hdPh/udimTextureObject.h"

#include "wabi/imaging/hio/glslfx.h"

#include "wabi/usd/sdr/declare.h"
#include "wabi/usd/sdr/registry.h"
#include "wabi/usd/sdr/shaderNode.h"
#include "wabi/usd/sdr/shaderProperty.h"

#include "wabi/usd/sdf/types.h"

#include "wabi/base/tf/envSetting.h"
#include "wabi/base/tf/hash.h"

#include <memory>

WABI_NAMESPACE_BEGIN

TF_DEFINE_PRIVATE_TOKENS(_tokens,
                         (opacity)(opacityThreshold)(isPtex)(st)(uv)(fieldname)(diffuseColor)(a)

                             (HwUvTexture_1)(textureMemory)(sourceColorSpace)(in)

                                 (mtlx));

static TfToken _GetMaterialTag(VtDictionary const &metadata, HdMaterialNode2 const &terminal)
{
  // Strongest materialTag opinion is a hardcoded tag in glslfx meta data.
  // This can be used for masked, additive, translucent or volume materials.
  // See HdMaterialTagTokens.
  VtValue vtMetaTag = TfMapLookupByValue(metadata, HdShaderTokens->materialTag, VtValue());

  if (vtMetaTag.IsHolding<std::string>()) {
    return TfToken(vtMetaTag.UncheckedGet<std::string>());
  }

  // Next check for authored terminal.opacityThreshold value > 0
  for (auto const &paramIt : terminal.parameters) {
    if (paramIt.first != _tokens->opacityThreshold)
      continue;

    VtValue const &vtOpacityThreshold = paramIt.second;
    if (vtOpacityThreshold.Get<float>() > 0.0f) {
      return HdPhMaterialTagTokens->masked;
    }
  }

  bool isTranslucent = false;

  // Next strongest opinion is a connection to 'terminal.opacity'
  auto const &opacityConIt = terminal.inputConnections.find(_tokens->opacity);
  isTranslucent            = (opacityConIt != terminal.inputConnections.end());

  // Weakest opinion is an authored terminal.opacity value.
  if (!isTranslucent) {
    for (auto const &paramIt : terminal.parameters) {
      if (paramIt.first != _tokens->opacity)
        continue;

      VtValue const &vtOpacity = paramIt.second;
      isTranslucent            = vtOpacity.Get<float>() < 1.0f;
      break;
    }
  }

  if (isTranslucent) {
    return HdPhMaterialTagTokens->translucent;
  }

  // An empty materialTag on the HdRprimCollection level means: 'ignore all
  // materialTags and add everything to the collection'. Instead we return a
  // default token because we want materialTags to drive HdPh collections.
  return HdPhMaterialTagTokens->defaultMaterialTag;
}

static void _GetGlslfxForTerminal(HioGlslfxSharedPtr &glslfxOut,
                                  size_t *glslfxOutHash,
                                  TfToken const &nodeTypeId,
                                  HdPhResourceRegistry *resourceRegistry)
{
  HD_TRACE_FUNCTION();

  // If there is a URI, we will use that, otherwise we will try to use
  // the source code.
  SdrRegistry &shaderReg        = SdrRegistry::GetInstance();
  SdrShaderNodeConstPtr sdrNode = shaderReg.GetShaderNodeByIdentifierAndType(
      nodeTypeId, HioGlslfxTokens->glslfx);

  if (sdrNode) {
    std::string const &glslfxFilePath = sdrNode->GetResolvedImplementationURI();
    if (!glslfxFilePath.empty()) {

      // Hash the filepath if it has changed.
      if (!(*glslfxOutHash) || (glslfxOut && glslfxOut->GetFilePath() != glslfxFilePath)) {
        *glslfxOutHash = TfHash()(glslfxFilePath);
      }

      // Find the glslfx file from the registry
      HdInstance<HioGlslfxSharedPtr> glslfxInstance = resourceRegistry->RegisterGLSLFXFile(
          *glslfxOutHash);

      if (glslfxInstance.IsFirstInstance()) {
        glslfxOut = std::make_shared<HioGlslfx>(glslfxFilePath);
        glslfxInstance.SetValue(glslfxOut);
      }
      glslfxOut = glslfxInstance.GetValue();
    }
    else {
      std::string const &sourceCode = sdrNode->GetSourceCode();
      if (!sourceCode.empty()) {
        // Do not use the registry for the source code to avoid
        // the cost of hashing the entire source code.
        std::istringstream sourceCodeStream(sourceCode);
        glslfxOut = std::make_shared<HioGlslfx>(sourceCodeStream);
      }
    }
  }
}

static HdMaterialNode2 const *_GetTerminalNode(HdMaterialNetwork2 const &network,
                                               TfToken const &terminalName,
                                               SdfPath *terminalNodePath)
{
  // Get the Surface or Volume Terminal
  auto const &terminalConnIt = network.terminals.find(terminalName);
  if (terminalConnIt == network.terminals.end()) {
    return nullptr;
  }
  HdMaterialConnection2 const &connection = terminalConnIt->second;
  SdfPath const &terminalPath             = connection.upstreamNode;
  auto const &terminalIt                  = network.nodes.find(terminalPath);
  *terminalNodePath                       = terminalPath;
  return &terminalIt->second;
}

// Get the fallback value for material node, first consulting Sdr to find
// whether the node has an input for the fallback value and then checking
// whether the output named outputName is known to Sdr and using either
// the default value specified by the SdrShaderProperty or using a
// default constructed value of the type specified by SdrShaderProperty.
//
static VtValue _GetNodeFallbackValue(HdMaterialNode2 const &node, TfToken const &outputName)
{
  SdrRegistry &shaderReg = SdrRegistry::GetInstance();

  // Find the corresponding Sdr node.
  SdrShaderNodeConstPtr const sdrNode = shaderReg.GetShaderNodeByIdentifierAndType(
      node.nodeTypeId, HioGlslfxTokens->glslfx);
  if (!sdrNode) {
    return VtValue();
  }

  // XXX Phoenix hack: Incorrect usage of GetDefaultInput to
  // determine what the fallback value is.
  // GetDefaultInput is meant to be used for 'disabled'
  // node where the 'default input' becomes the value
  // pass-through in the network. But Phoenix has no other
  // mechanism currently to deal with fallback values.
  if (SdrShaderPropertyConstPtr const &defaultInput = sdrNode->GetDefaultInput()) {
    TfToken const &def     = defaultInput->GetName();
    auto const &defParamIt = node.parameters.find(def);
    if (defParamIt != node.parameters.end()) {
      return defParamIt->second;
    }
  }

  // Sdr supports specifying default values for outputs so if we
  // did not use the GetDefaultInput hack above, we fallback to
  // using this DefaultOutput value.
  if (SdrShaderPropertyConstPtr const &output = sdrNode->GetShaderOutput(outputName)) {
    const VtValue out = output->GetDefaultValue();
    if (!out.IsEmpty()) {
      return out;
    }

    // If no default value was registered with Sdr for
    // the output, fallback to the type's default.
    return output->GetTypeAsSdfType().first.GetDefaultValue();
  }

  return VtValue();
}

static VtValue _GetParamFallbackValue(HdMaterialNetwork2 const &network,
                                      HdMaterialNode2 const &node,
                                      TfToken const &paramName)
{
  // The 'fallback value' will be the value of the material param if nothing
  // is connected or what is connected is mis-configured. For example a
  // missing texture file.

  SdrRegistry &shaderReg = SdrRegistry::GetInstance();

  // Check if there are any connections to the terminal input.
  auto const &connIt = node.inputConnections.find(paramName);

  if (connIt != node.inputConnections.end()) {
    if (!connIt->second.empty()) {
      HdMaterialConnection2 const &con    = connIt->second.front();
      auto const &pnIt                    = network.nodes.find(con.upstreamNode);
      HdMaterialNode2 const &upstreamNode = pnIt->second;

      const VtValue fallbackValue = _GetNodeFallbackValue(upstreamNode, con.upstreamOutputName);
      if (!fallbackValue.IsEmpty()) {
        return fallbackValue;
      }
    }
  }

  // If there are no connections there may be an authored value.

  auto const &it = node.parameters.find(paramName);
  if (it != node.parameters.end()) {
    return it->second;
  }

  // If we had nothing connected, but we do have an Sdr node we can use the
  // DefaultValue for the input as specified in the Sdr schema.
  // E.g. PreviewSurface is a terminal with an Sdr schema.

  SdrShaderNodeConstPtr terminalSdr = shaderReg.GetShaderNodeByIdentifierAndType(
      node.nodeTypeId, HioGlslfxTokens->glslfx);

  if (terminalSdr) {
    if (SdrShaderPropertyConstPtr const &input = terminalSdr->GetShaderInput(paramName)) {
      VtValue out = input->GetDefaultValue();
      // If not default value was registered with Sdr for
      // the output, fallback to the type's default.
      if (out.IsEmpty()) {
        out = input->GetTypeAsSdfType().first.GetDefaultValue();
      }

      if (!out.IsEmpty())
        return out;
    }
  }

  // Returning an empty value will likely result in a shader compile error,
  // because the buffer source will not be able to determine the HdTupleType.
  // Hope for the best and return a vec3.

  TF_WARN("Couldn't determine default value for: %s on nodeType: %s",
          paramName.GetText(),
          node.nodeTypeId.GetText());

  return VtValue(GfVec3f(0));
}

static TfToken _GetPrimvarNameAttributeValue(SdrShaderNodeConstPtr const &sdrNode,
                                             HdMaterialNode2 const &node,
                                             TfToken const &propName)
{
  VtValue vtName;

  // If the name of the primvar was authored, the material adapter would have
  // put that that authored value in the node's parameter list.
  // The authored value is the strongest opinion/

  auto const &paramIt = node.parameters.find(propName);
  if (paramIt != node.parameters.end()) {
    vtName = paramIt->second;
  }

  // If we didn't find an authored value consult Sdr for the default value.
  if (vtName.IsEmpty() && sdrNode) {
    if (SdrShaderPropertyConstPtr sdrPrimvarInput = sdrNode->GetShaderInput(propName)) {
      vtName = sdrPrimvarInput->GetDefaultValue();
    }
  }

  if (vtName.IsHolding<TfToken>()) {
    return vtName.UncheckedGet<TfToken>();
  }
  else if (vtName.IsHolding<std::string>()) {
    return TfToken(vtName.UncheckedGet<std::string>());
  }

  return TfToken();
}

static void _MakeMaterialParamsForUnconnectedParam(TfToken const &paramName,
                                                   HdPh_MaterialParamVector *params)
{
  HdPh_MaterialParam param;
  param.paramType = HdPh_MaterialParam::ParamTypeFallback;
  param.name      = paramName;

  params->push_back(std::move(param));
}

static void _MakeMaterialParamsForAdditionalPrimvar(TfToken const &primvarName,
                                                    HdPh_MaterialParamVector *params)
{
  HdPh_MaterialParam param;
  param.paramType = HdPh_MaterialParam::ParamTypeAdditionalPrimvar;
  param.name      = primvarName;

  params->push_back(std::move(param));
}

static void _MakeMaterialParamsForPrimvarReader(HdMaterialNetwork2 const &network,
                                                HdMaterialNode2 const &node,
                                                SdfPath const &nodePath,
                                                TfToken const &paramName,
                                                SdfPathSet *visitedNodes,
                                                HdPh_MaterialParamVector *params)
{
  if (visitedNodes->find(nodePath) != visitedNodes->end())
    return;

  SdrRegistry &shaderReg        = SdrRegistry::GetInstance();
  SdrShaderNodeConstPtr sdrNode = shaderReg.GetShaderNodeByIdentifierAndType(
      node.nodeTypeId, HioGlslfxTokens->glslfx);

  HdPh_MaterialParam param;
  param.paramType = HdPh_MaterialParam::ParamTypePrimvarRedirect;
  param.name      = paramName;

  // A node may require 'additional primvars' to function correctly.
  for (auto const &propName : sdrNode->GetAdditionalPrimvarProperties()) {
    TfToken primvarName = _GetPrimvarNameAttributeValue(sdrNode, node, propName);

    if (!primvarName.IsEmpty()) {
      param.samplerCoords.push_back(primvarName);
    }
  }

  params->push_back(std::move(param));
}

static void _MakeMaterialParamsForTransform2d(HdMaterialNetwork2 const &network,
                                              HdMaterialNode2 const &node,
                                              SdfPath const &nodePath,
                                              TfToken const &paramName,
                                              SdfPathSet *visitedNodes,
                                              HdPh_MaterialParamVector *params)
{
  if (visitedNodes->find(nodePath) != visitedNodes->end())
    return;

  SdrRegistry &shaderReg = SdrRegistry::GetInstance();

  HdPh_MaterialParam transform2dParam;
  transform2dParam.paramType     = HdPh_MaterialParam::ParamTypeTransform2d;
  transform2dParam.name          = paramName;
  transform2dParam.fallbackValue = _GetParamFallbackValue(network, node, _tokens->in);

  HdPh_MaterialParamVector additionalParams;

  // Find the input connection to the transform2d node
  auto inIt = node.inputConnections.find(_tokens->in);
  if (inIt != node.inputConnections.end()) {
    if (!inIt->second.empty()) {
      HdMaterialConnection2 const &con = inIt->second.front();
      SdfPath const &upstreamNodePath  = con.upstreamNode;

      auto const &pnIt                   = network.nodes.find(upstreamNodePath);
      HdMaterialNode2 const &primvarNode = pnIt->second;
      SdrShaderNodeConstPtr primvarSdr   = shaderReg.GetShaderNodeByIdentifierAndType(
          primvarNode.nodeTypeId, HioGlslfxTokens->glslfx);

      if (primvarSdr) {
        HdPh_MaterialParamVector primvarParams;

        _MakeMaterialParamsForPrimvarReader(
            network, primvarNode, upstreamNodePath, inIt->first, visitedNodes, &primvarParams);

        if (!primvarParams.empty()) {
          HdPh_MaterialParam const &primvarParam = primvarParams.front();
          // Extract the referenced primvar(s) to go into the
          // transform2d's sampler coords.
          transform2dParam.samplerCoords = primvarParam.samplerCoords;
        }

        // Make sure we add any referenced primvars as "additional
        // primvars" so they make it through primvar filtering.
        for (auto const &primvarName : transform2dParam.samplerCoords) {
          _MakeMaterialParamsForAdditionalPrimvar(primvarName, &additionalParams);
        }
      }
    }
  }
  else {
    // See if input value was directly authored as value.
    auto iter = node.parameters.find(_tokens->in);

    if (iter != node.parameters.end()) {
      if (iter->second.IsHolding<TfToken>()) {
        TfToken const &samplerCoord = iter->second.UncheckedGet<TfToken>();
        transform2dParam.samplerCoords.push_back(samplerCoord);
      }
    }
  }

  params->push_back(std::move(transform2dParam));

  // Make materials params for each component of transform2d
  // (rotation, scale, translation)
  HdPh_MaterialParam rotParam;
  rotParam.paramType     = HdPh_MaterialParam::ParamTypeFallback;
  rotParam.name          = TfToken(paramName.GetString() + "_" + HdPhTokens->rotation.GetString());
  rotParam.fallbackValue = _GetParamFallbackValue(network, node, HdPhTokens->rotation);
  params->push_back(std::move(rotParam));

  HdPh_MaterialParam scaleParam;
  scaleParam.paramType     = HdPh_MaterialParam::ParamTypeFallback;
  scaleParam.name          = TfToken(paramName.GetString() + "_" + HdPhTokens->scale.GetString());
  scaleParam.fallbackValue = _GetParamFallbackValue(network, node, HdPhTokens->scale);
  params->push_back(std::move(scaleParam));

  HdPh_MaterialParam transParam;
  transParam.paramType = HdPh_MaterialParam::ParamTypeFallback;
  transParam.name = TfToken(paramName.GetString() + "_" + HdPhTokens->translation.GetString());
  transParam.fallbackValue = _GetParamFallbackValue(network, node, HdPhTokens->translation);
  params->push_back(std::move(transParam));

  // Need to add these at the end because the caller expects the
  // "transform" param to be first.
  params->insert(params->end(), additionalParams.begin(), additionalParams.end());
}

static std::string _ResolveAssetPath(VtValue const &value)
{
  // Note that the SdfAssetPath should really be resolved into an ArAsset via
  // ArGetResolver (Eg. USDZ). Using GetResolvePath directly isn't sufficient.
  // Texture loading in Phoenix goes via Glf, which will handle the ArAsset
  // resolution already, so we skip doing it here and simply use the string.
  if (value.IsHolding<SdfAssetPath>()) {
    SdfAssetPath p = value.Get<SdfAssetPath>();
    std::string v  = p.GetResolvedPath();
    if (v.empty()) {
      v = p.GetAssetPath();
    }
    return v;
  }
  else if (value.IsHolding<std::string>()) {
    return value.UncheckedGet<std::string>();
  }

  return std::string();
}

// Look up value from material node parameters and fallback to
// corresponding value on given SdrNode.
template<typename T>
static auto _ResolveParameter(HdMaterialNode2 const &node,
                              SdrShaderNodeConstPtr const &sdrNode,
                              TfToken const &name,
                              T const &defaultValue) -> T
{
  // First consult node parameters...
  const auto it = node.parameters.find(name);
  if (it != node.parameters.end()) {
    const VtValue &value = it->second;
    if (value.IsHolding<T>()) {
      return value.UncheckedGet<T>();
    }
  }

  // Then fallback to SdrNode.
  if (sdrNode) {
    if (SdrShaderPropertyConstPtr const input = sdrNode->GetShaderInput(name)) {
      const VtValue &value = input->GetDefaultValue();
      if (value.IsHolding<T>()) {
        return value.UncheckedGet<T>();
      }
    }
  }

  return defaultValue;
}

static HdWrap _ResolveWrapSamplerParameter(SdfPath const &nodePath,
                                           HdMaterialNode2 const &node,
                                           SdrShaderNodeConstPtr const &sdrNode,
                                           TfToken const &name)
{
  const TfToken value = _ResolveParameter(node, sdrNode, name, HdPhTextureTokens->useMetadata);

  if (value == HdPhTextureTokens->repeat) {
    return HdWrapRepeat;
  }

  if (value == HdPhTextureTokens->mirror) {
    return HdWrapMirror;
  }

  if (value == HdPhTextureTokens->clamp) {
    return HdWrapClamp;
  }

  if (value == HdPhTextureTokens->black) {
    return HdWrapBlack;
  }

  if (value == HdPhTextureTokens->useMetadata) {
    if (node.nodeTypeId == _tokens->HwUvTexture_1) {
      return HdWrapLegacy;
    }
    return HdWrapUseMetadata;
  }

  TF_WARN("Unknown wrap mode on prim %s: %s", nodePath.GetText(), value.GetText());

  return HdWrapUseMetadata;
}

static HdMinFilter _ResolveMinSamplerParameter(SdfPath const &nodePath,
                                               HdMaterialNode2 const &node,
                                               SdrShaderNodeConstPtr const &sdrNode)
{
  // Using linearMipmapLinear as fallback value.

  // Note that it is ambiguous whether the fallback value in the old
  // texture system (usdImagingGL/textureUtils.cpp) was linear or
  // linearMipmapLinear: when nothing was authored in USD for the
  // min filter, linearMipmapLinear was used, but when an empty
  // token was authored, linear was used.

  const TfToken value = _ResolveParameter(
      node, sdrNode, HdPhTextureTokens->minFilter, HdPhTextureTokens->linearMipmapLinear);

  if (value == HdPhTextureTokens->nearest) {
    return HdMinFilterNearest;
  }

  if (value == HdPhTextureTokens->linear) {
    return HdMinFilterLinear;
  }

  if (value == HdPhTextureTokens->nearestMipmapNearest) {
    return HdMinFilterNearestMipmapNearest;
  }

  if (value == HdPhTextureTokens->nearestMipmapLinear) {
    return HdMinFilterNearestMipmapLinear;
  }

  if (value == HdPhTextureTokens->linearMipmapNearest) {
    return HdMinFilterLinearMipmapNearest;
  }

  if (value == HdPhTextureTokens->linearMipmapLinear) {
    return HdMinFilterLinearMipmapLinear;
  }

  return HdMinFilterLinearMipmapLinear;
}

static HdMagFilter _ResolveMagSamplerParameter(SdfPath const &nodePath,
                                               HdMaterialNode2 const &node,
                                               SdrShaderNodeConstPtr const &sdrNode)
{
  const TfToken value = _ResolveParameter(
      node, sdrNode, HdPhTextureTokens->magFilter, HdPhTextureTokens->linear);

  if (value == HdPhTextureTokens->nearest) {
    return HdMagFilterNearest;
  }

  return HdMagFilterLinear;
}

// Resolve sampling parameters for texture node by
// looking at material node parameters and falling back to
// fallback values from Sdr.
static HdSamplerParameters _GetSamplerParameters(SdfPath const &nodePath,
                                                 HdMaterialNode2 const &node,
                                                 SdrShaderNodeConstPtr const &sdrNode)
{
  return {_ResolveWrapSamplerParameter(nodePath, node, sdrNode, HdPhTextureTokens->wrapS),
          _ResolveWrapSamplerParameter(nodePath, node, sdrNode, HdPhTextureTokens->wrapT),
          _ResolveWrapSamplerParameter(nodePath, node, sdrNode, HdPhTextureTokens->wrapR),
          _ResolveMinSamplerParameter(nodePath, node, sdrNode),
          _ResolveMagSamplerParameter(nodePath, node, sdrNode)};
}

//
// We need to flip the image for the legacy HwUvTexture_1 shader node,
// pre-multiply textures by their alpha if applicable, and provide a hint for
// in what color space the texture is encoded
//
static std::unique_ptr<HdPhSubtextureIdentifier> _GetSubtextureIdentifier(
    const HdTextureType textureType,
    const TfToken &nodeType,
    const bool premultiplyAlpha,
    const TfToken &sourceColorSpace)
{
  if (textureType == HdTextureType::Uv) {
    const bool flipVertically = (nodeType == _tokens->HwUvTexture_1);
    return std::make_unique<HdPhAssetUvSubtextureIdentifier>(
        flipVertically, premultiplyAlpha, sourceColorSpace);
  }
  if (textureType == HdTextureType::Udim) {
    return std::make_unique<HdPhUdimSubtextureIdentifier>(premultiplyAlpha, sourceColorSpace);
  }
  if (textureType == HdTextureType::Ptex) {
    return std::make_unique<HdPhPtexSubtextureIdentifier>(premultiplyAlpha);
  }
  return nullptr;
}

static void _MakeMaterialParamsForTexture(
    HdMaterialNetwork2 const &network,
    HdMaterialNode2 const &node,
    HdMaterialNode2 const &downstreamNode,  // needed to determine def value
    SdfPath const &nodePath,
    TfToken const &outputName,
    TfToken const &paramName,
    SdfPathSet *visitedNodes,
    HdPh_MaterialParamVector *params,
    HdPhMaterialNetwork::TextureDescriptorVector *textureDescriptors,
    TfToken const &materialTag)
{
  if (visitedNodes->find(nodePath) != visitedNodes->end())
    return;

  SdrRegistry &shaderReg        = SdrRegistry::GetInstance();
  SdrShaderNodeConstPtr sdrNode = shaderReg.GetShaderNodeByIdentifier(
      node.nodeTypeId, {HioGlslfxTokens->glslfx, _tokens->mtlx});

  HdPh_MaterialParam texParam;
  texParam.paramType = HdPh_MaterialParam::ParamTypeTexture;
  texParam.name      = paramName;

  // Get swizzle metadata if possible
  if (SdrShaderPropertyConstPtr sdrProperty = sdrNode->GetShaderOutput(outputName)) {
    NdrTokenMap const &propMetadata = sdrProperty->GetMetadata();
    auto const &it                  = propMetadata.find(HdPhSdrMetadataTokens->swizzle);
    if (it != propMetadata.end()) {
      texParam.swizzle = it->second;
    }
  }

  // Determine the texture type
  texParam.textureType = HdTextureType::Uv;
  if (sdrNode && sdrNode->GetMetadata().count(_tokens->isPtex)) {
    texParam.textureType = HdTextureType::Ptex;
  }

  // Determine if texture should be pre-multiplied on CPU
  // Currently, this will only happen if the texture param is called
  // "diffuseColor" and if there is another param "opacity" connected to the
  // same texture node via output "a", as long as the material tag is not
  // "masked"
  bool premultiplyTexture = false;
  if (paramName == _tokens->diffuseColor && materialTag != HdPhMaterialTagTokens->masked) {
    auto const &opacityConIt = downstreamNode.inputConnections.find(_tokens->opacity);
    if (opacityConIt != downstreamNode.inputConnections.end()) {
      HdMaterialConnection2 const &con = opacityConIt->second.front();
      premultiplyTexture               = ((nodePath == con.upstreamNode) &&
                            (con.upstreamOutputName == _tokens->a));
    }
  }
  texParam.isPremultiplied = premultiplyTexture;

  // Get texture's sourceColorSpace hint
  const TfToken sourceColorSpace = _ResolveParameter(
      node, sdrNode, _tokens->sourceColorSpace, HdPhTokens->colorSpaceAuto);

  // Extract texture file path
  bool useTexturePrimToFindTexture = true;

  SdfPath texturePrimPathForSceneDelegate;

  HdPhTextureIdentifier textureId;

  NdrTokenVec const &assetIdentifierPropertyNames = sdrNode->GetAssetIdentifierInputNames();

  if (assetIdentifierPropertyNames.size() == 1) {
    TfToken const &fileProp = assetIdentifierPropertyNames[0];
    auto const &it          = node.parameters.find(fileProp);
    if (it != node.parameters.end()) {
      const VtValue &v = it->second;
      // We use the nodePath, not the filePath, for the 'connection'.
      // Based on the connection path we will do a texture lookup via
      // the scene delegate. The scene delegate will lookup this texture
      // prim (by path) to query the file attribute value for filepath.
      // The reason for this re-direct is to support other texture uses
      // such as render-targets.
      texturePrimPathForSceneDelegate = nodePath;

      // Use the type of the filePath attribute to determine
      // whether to use the Phoenix texture system (for
      // SdfAssetPath/std::string/ HdPhTextureIdentifier) or use
      // the render buffer associated to a draw target.
      //
      if (v.IsHolding<HdPhTextureIdentifier>()) {
        //
        // Clients can explicitly give an HdPhTextureIdentifier for
        // more direct control since they can give an instance of
        // HdPhSubtextureIdentifier.
        //
        // Examples are, e.g., HdPhUvAssetSubtextureIdentifier
        // allowing clients to flip the texture. Clients can even
        // subclass from HdPhDynamicUvSubtextureIdentifier and
        // HdPhDynamicUvTextureImplementation to implement their own
        // texture loading and commit.
        //
        useTexturePrimToFindTexture = false;
        textureId                   = v.UncheckedGet<HdPhTextureIdentifier>();
      }
      else if (v.IsHolding<std::string>() || v.IsHolding<SdfAssetPath>()) {
        const std::string filePath = _ResolveAssetPath(v);

        if (HdPhIsSupportedUdimTexture(filePath)) {
          texParam.textureType = HdTextureType::Udim;
        }

        useTexturePrimToFindTexture = false;
        textureId                   = HdPhTextureIdentifier(
            TfToken(filePath),
            _GetSubtextureIdentifier(
                texParam.textureType, node.nodeTypeId, premultiplyTexture, sourceColorSpace));
        // If the file attribute is an SdfPath, interpret it as path
        // to a prim holding the texture resource (e.g., a render buffer).
      }
      else if (v.IsHolding<SdfPath>()) {
        texturePrimPathForSceneDelegate = v.UncheckedGet<SdfPath>();
      }
    }
  }
  else {
    TF_WARN("Invalid number of asset identifier input names: %s", nodePath.GetText());
  }

  // Check to see if a primvar or transform2d node is connected to 'st' or
  // 'uv'.
  // Instead of looking for a st inputs by name we could traverse all
  // connections to inputs and pick one that has a 'primvar' or 'transform2d'
  // node attached. That could also be problematic if you connect a primvar or
  // transform2d to one of the other inputs of the texture node.
  auto stIt = node.inputConnections.find(_tokens->st);
  if (stIt == node.inputConnections.end()) {
    stIt = node.inputConnections.find(_tokens->uv);
  }

  if (stIt != node.inputConnections.end()) {
    if (!stIt->second.empty()) {
      HdMaterialConnection2 const &con = stIt->second.front();
      SdfPath const &upstreamNodePath  = con.upstreamNode;

      auto const &upIt                    = network.nodes.find(upstreamNodePath);
      HdMaterialNode2 const &upstreamNode = upIt->second;

      SdrShaderNodeConstPtr upstreamSdr = shaderReg.GetShaderNodeByIdentifierAndType(
          upstreamNode.nodeTypeId, HioGlslfxTokens->glslfx);

      if (upstreamSdr) {
        TfToken sdrRole(upstreamSdr->GetRole());
        if (sdrRole == SdrNodeRole->Primvar) {
          HdPh_MaterialParamVector primvarParams;

          _MakeMaterialParamsForPrimvarReader(
              network, upstreamNode, upstreamNodePath, stIt->first, visitedNodes, &primvarParams);

          if (!primvarParams.empty()) {
            HdPh_MaterialParam const &primvarParam = primvarParams.front();
            // Extract the referenced primvar(s) for use in the texture
            // sampler coords.
            texParam.samplerCoords = primvarParam.samplerCoords;
          }

          // For any referenced primvars, add them as "additional primvars"
          // to make sure they pass primvar filtering.
          for (auto const &primvarName : texParam.samplerCoords) {
            _MakeMaterialParamsForAdditionalPrimvar(primvarName, params);
          }
        }
        else if (sdrRole == SdrNodeRole->Math) {
          HdPh_MaterialParamVector transform2dParams;

          _MakeMaterialParamsForTransform2d(
              network,
              upstreamNode,
              upstreamNodePath,
              TfToken(paramName.GetString() + "_" + stIt->first.GetString() + "_transform2d"),
              visitedNodes,
              &transform2dParams);

          if (!transform2dParams.empty()) {
            HdPh_MaterialParam const &transform2dParam = transform2dParams.front();
            // The texure's sampler coords should come from the
            // output of the transform2d
            texParam.samplerCoords.push_back(transform2dParam.name);
          }

          // Copy params created for tranform2d node to param list
          params->insert(params->end(), transform2dParams.begin(), transform2dParams.end());
        }
      }
    }
  }
  else {

    // See if a st value was directly authored as value.

    auto iter = node.parameters.find(_tokens->st);
    if (iter == node.parameters.end()) {
      iter = node.parameters.find(_tokens->uv);
    }

    if (iter != node.parameters.end()) {
      if (iter->second.IsHolding<TfToken>()) {
        TfToken const &samplerCoord = iter->second.UncheckedGet<TfToken>();
        texParam.samplerCoords.push_back(samplerCoord);
      }
    }
  }

  // Handle texture scale and bias
  HdPh_MaterialParam texScaleParam;
  texScaleParam.paramType = HdPh_MaterialParam::ParamTypeFallback;
  texScaleParam.name      = TfToken(paramName.GetString() + "_" + HdPhTokens->scale.GetString());
  texScaleParam.fallbackValue = VtValue(
      _ResolveParameter(node, sdrNode, HdPhTokens->scale, GfVec4f(1.0f)));
  params->push_back(std::move(texScaleParam));

  HdPh_MaterialParam texBiasParam;
  texBiasParam.paramType     = HdPh_MaterialParam::ParamTypeFallback;
  texBiasParam.name          = TfToken(paramName.GetString() + "_" + HdPhTokens->bias.GetString());
  texBiasParam.fallbackValue = VtValue(
      _ResolveParameter(node, sdrNode, HdPhTokens->bias, GfVec4f(0.0f)));
  params->push_back(std::move(texBiasParam));

  // Attribute is in Mebibytes, but Phoenix texture system expects
  // bytes.
  const size_t memoryRequest = 1048576 * _ResolveParameter<float>(
                                             node, sdrNode, _tokens->textureMemory, 0.0f);

  textureDescriptors->push_back({paramName,
                                 textureId,
                                 texParam.textureType,
                                 _GetSamplerParameters(nodePath, node, sdrNode),
                                 memoryRequest,
                                 useTexturePrimToFindTexture,
                                 texturePrimPathForSceneDelegate});

  params->push_back(std::move(texParam));
}

static void _MakeMaterialParamsForFieldReader(HdMaterialNetwork2 const &network,
                                              HdMaterialNode2 const &node,
                                              SdfPath const &nodePath,
                                              TfToken const &paramName,
                                              SdfPathSet *visitedNodes,
                                              HdPh_MaterialParamVector *params)
{
  if (visitedNodes->find(nodePath) != visitedNodes->end())
    return;

  // Volume Fields act more like a primvar then a texture.
  // There is a `Volume` prim with 'fields' that may point to a
  // OpenVDB file. We have to find the 'inputs:fieldname' on the
  // HWFieldReader in the material network to know what 'field' to use.
  // See also HdPhVolume and HdPhField for how volume textures are
  // inserted into Phoenix.

  HdPh_MaterialParam param;
  param.paramType = HdPh_MaterialParam::ParamTypeFieldRedirect;
  param.name      = paramName;

  // XXX Why _tokens->fieldname:
  // Hard-coding the name of the attribute of HwFieldReader identifying
  // the field name for now.
  // The equivalent of the generic mechanism Sdr provides for primvars
  // is missing for fields: UsdPrimvarReader.inputs:varname is tagged with
  // sdrMetadata as primvarProperty="1" so that we can use
  // sdrNode->GetAdditionalPrimvarProperties to know what attribute to use.
  TfToken const &varName = _tokens->fieldname;

  auto const &it = node.parameters.find(varName);
  if (it != node.parameters.end()) {
    VtValue fieldName = it->second;
    if (fieldName.IsHolding<TfToken>()) {
      // Stashing name of field in _samplerCoords.
      param.samplerCoords.push_back(fieldName.UncheckedGet<TfToken>());
    }
    else if (fieldName.IsHolding<std::string>()) {
      param.samplerCoords.push_back(TfToken(fieldName.UncheckedGet<std::string>()));
    }
  }

  params->push_back(std::move(param));
}

static void _MakeParamsForInputParameter(
    HdMaterialNetwork2 const &network,
    HdMaterialNode2 const &node,
    TfToken const &paramName,
    SdfPathSet *visitedNodes,
    HdPh_MaterialParamVector *params,
    HdPhMaterialNetwork::TextureDescriptorVector *textureDescriptors,
    TfToken const &materialTag)
{
  SdrRegistry &shaderReg = SdrRegistry::GetInstance();

  // Resolve what is connected to this param (eg. primvar, texture, nothing)
  // and then make the correct HdPh_MaterialParam for it.
  auto const &conIt = node.inputConnections.find(paramName);

  if (conIt != node.inputConnections.end()) {

    std::vector<HdMaterialConnection2> const &cons = conIt->second;
    if (!cons.empty()) {

      // Find the node that is connected to this input
      HdMaterialConnection2 const &con = cons.front();
      auto const &upIt                 = network.nodes.find(con.upstreamNode);

      if (upIt != network.nodes.end()) {

        SdfPath const &upstreamPath         = upIt->first;
        TfToken const &upstreamOutputName   = con.upstreamOutputName;
        HdMaterialNode2 const &upstreamNode = upIt->second;

        SdrShaderNodeConstPtr upstreamSdr = shaderReg.GetShaderNodeByIdentifier(
            upstreamNode.nodeTypeId, {HioGlslfxTokens->glslfx, _tokens->mtlx});

        if (upstreamSdr) {
          TfToken sdrRole(upstreamSdr->GetRole());
          if (sdrRole == SdrNodeRole->Texture) {
            _MakeMaterialParamsForTexture(network,
                                          upstreamNode,
                                          node,
                                          upstreamPath,
                                          upstreamOutputName,
                                          paramName,
                                          visitedNodes,
                                          params,
                                          textureDescriptors,
                                          materialTag);
            return;
          }
          else if (sdrRole == SdrNodeRole->Primvar) {
            _MakeMaterialParamsForPrimvarReader(
                network, upstreamNode, upstreamPath, paramName, visitedNodes, params);
            return;
          }
          else if (sdrRole == SdrNodeRole->Field) {
            _MakeMaterialParamsForFieldReader(
                network, upstreamNode, upstreamPath, paramName, visitedNodes, params);
            return;
          }
          else if (sdrRole == SdrNodeRole->Math) {
            _MakeMaterialParamsForTransform2d(
                network, upstreamNode, upstreamPath, paramName, visitedNodes, params);
            return;
          }
        }
        else {
          TF_WARN("Unrecognized connected node: %s", upstreamNode.nodeTypeId.GetText());
        }
      }
    }
  }

  // Nothing (supported) was connected, output a fallback material param
  _MakeMaterialParamsForUnconnectedParam(paramName, params);
}

static void _GatherMaterialParams(HdMaterialNetwork2 const &network,
                                  HdMaterialNode2 const &node,
                                  HdPh_MaterialParamVector *params,
                                  HdPhMaterialNetwork::TextureDescriptorVector *textureDescriptors,
                                  TfToken const &materialTag)
{
  HD_TRACE_FUNCTION();

  // Hydra Phoenix currently supports two material configurations.
  // A custom glslfx file or a PreviewSurface material network.
  // Either configuration consists of a terminal (Shader or PreviewSurface)
  // with its input values authored or connected to a primvar, texture or
  // volume node. The texture may have a primvar connected to provide UVs.
  //
  // The following code is made to process one of these two material configs
  // exclusively. It cannot convert arbitrary material networks to Phoenix by
  // generating the appropriate glsl code.

  SdrRegistry &shaderReg        = SdrRegistry::GetInstance();
  SdrShaderNodeConstPtr sdrNode = shaderReg.GetShaderNodeByIdentifierAndType(
      node.nodeTypeId, HioGlslfxTokens->glslfx);

  SdfPathSet visitedNodes;

  TfTokenVector parameters;
  if (sdrNode) {
    parameters = sdrNode->GetInputNames();
  }
  else {
    TF_WARN("Unrecognized node: %s", node.nodeTypeId.GetText());
  }

  for (TfToken const &inputName : parameters) {
    _MakeParamsForInputParameter(
        network, node, inputName, &visitedNodes, params, textureDescriptors, materialTag);
  }

  // Set fallback values for the inputs on the terminal (excepting
  // referenced sampler coords).
  for (HdPh_MaterialParam &p : *params) {
    if (p.paramType != HdPh_MaterialParam::ParamTypeAdditionalPrimvar &&
        p.fallbackValue.IsEmpty()) {
      p.fallbackValue = _GetParamFallbackValue(network, node, p.name);
    }
  }

  // Create HdPh_MaterialParams for each primvar the terminal says it needs.
  // Primvars come from 'attributes' in the glslfx and are seperate from
  // the input 'parameters'. We need to create a material param for them so
  // that these primvars survive 'primvar filtering' that discards any unused
  // primvars on the mesh.
  // If the network lists additional primvars, we add those too.
  NdrTokenVec pv = sdrNode->GetPrimvars();
  pv.insert(pv.end(), network.primvars.begin(), network.primvars.end());
  std::sort(pv.begin(), pv.end());
  pv.erase(std::unique(pv.begin(), pv.end()), pv.end());

  for (TfToken const &primvarName : pv) {
    _MakeMaterialParamsForAdditionalPrimvar(primvarName, params);
  }
}

HdPhMaterialNetwork::HdPhMaterialNetwork()
    : _materialTag(HdPhMaterialTagTokens->defaultMaterialTag),
      _surfaceGfxHash(0)
{}

HdPhMaterialNetwork::~HdPhMaterialNetwork() = default;

void HdPhMaterialNetwork::ProcessMaterialNetwork(SdfPath const &materialId,
                                                 HdMaterialNetworkMap const &hdNetworkMap,
                                                 HdPhResourceRegistry *resourceRegistry)
{
  HD_TRACE_FUNCTION();

  _fragmentSource.clear();
  _geometrySource.clear();
  _materialMetadata.clear();
  _materialParams.clear();
  _textureDescriptors.clear();
  _materialTag = HdPhMaterialTagTokens->defaultMaterialTag;

  HdMaterialNetwork2 surfaceNetwork;

  // The fragment source comes from the 'surface' network or the
  // 'volume' network.
  bool isVolume = false;
  HdMaterialNetwork2ConvertFromHdMaterialNetworkMap(hdNetworkMap, &surfaceNetwork, &isVolume);
  const TfToken &terminalName = (isVolume) ? HdMaterialTerminalTokens->volume :
                                             HdMaterialTerminalTokens->surface;

  SdfPath surfTerminalPath;
  if (HdMaterialNode2 const *surfTerminal = _GetTerminalNode(
          surfaceNetwork, terminalName, &surfTerminalPath)) {

#ifdef WITH_MATERIALX
    if (!isVolume) {
      HdPh_ApplyMaterialXFilter(&surfaceNetwork, materialId, *surfTerminal, surfTerminalPath);
    }
#endif
    // Extract the glslfx and metadata for surface/volume.
    _GetGlslfxForTerminal(
        _surfaceGfx, &_surfaceGfxHash, surfTerminal->nodeTypeId, resourceRegistry);
    if (_surfaceGfx) {

      // If the glslfx file is not valid we skip parsing the network.
      // This produces no fragmentSource which means Phoenix's material
      // will use the fallback shader.
      if (_surfaceGfx->IsValid()) {

        _fragmentSource   = isVolume ? _surfaceGfx->GetVolumeSource() :
                                       _surfaceGfx->GetSurfaceSource();
        _materialMetadata = _surfaceGfx->GetMetadata();
        _materialTag      = _GetMaterialTag(_materialMetadata, *surfTerminal);
        _GatherMaterialParams(
            surfaceNetwork, *surfTerminal, &_materialParams, &_textureDescriptors, _materialTag);

        // OSL networks have a displacement network in hdNetworkMap
        // under terminal: HdMaterialTerminalTokens->displacement.
        // For Phoenix however we expect the displacement shader to be
        // provided via the surface glslfx / terminal.
        _geometrySource = _surfaceGfx->GetDisplacementSource();
      }
    }
  }
}

TfToken const &HdPhMaterialNetwork::GetMaterialTag() const
{
  return _materialTag;
}

std::string const &HdPhMaterialNetwork::GetFragmentCode() const
{
  return _fragmentSource;
}

std::string const &HdPhMaterialNetwork::GetGeometryCode() const
{
  return _geometrySource;
}

VtDictionary const &HdPhMaterialNetwork::GetMetadata() const
{
  return _materialMetadata;
}

HdPh_MaterialParamVector const &HdPhMaterialNetwork::GetMaterialParams() const
{
  return _materialParams;
}

HdPhMaterialNetwork::TextureDescriptorVector const &HdPhMaterialNetwork::GetTextureDescriptors()
    const
{
  return _textureDescriptors;
}

WABI_NAMESPACE_END
