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

#include "wabi/usd/ndr/node.h"
#include "wabi/usd/ndr/nodeDiscoveryResult.h"
#include "wabi/usd/ndr/parserPlugin.h"

#include "wabi/base/tf/staticTokens.h"

WABI_NAMESPACE_BEGIN

TF_DEFINE_PRIVATE_TOKENS(_tokens, (rpr));

/// \class HdRprNdrParserPlugin
///
/// This does the minimal amount of work so Hydra will let us have our shaders.
///
class HdRprNdrParserPlugin final : public NdrParserPlugin
{
 public:
  NdrNodeUniquePtr Parse(const NdrNodeDiscoveryResult &discoveryResult) override
  {
    return std::make_unique<NdrNode>(
      /* identifier  = */ discoveryResult.identifier,
      /* version     = */ discoveryResult.version,
      /* name        = */ discoveryResult.name,
      /* family      = */ discoveryResult.family,
      /* context     = */ _tokens->rpr,
      /* sourceType  = */ _tokens->rpr,
      /* uri         = */ discoveryResult.uri,
#if WABI_VERSION > 1911
      /* resolvedUri = */ discoveryResult.resolvedUri,
#endif
      /* properties  = */ NdrPropertyUniquePtrVec{},
      /* metadata    = */ discoveryResult.metadata,
      /* sourceCode  = */ discoveryResult.sourceCode);
  }

  const NdrTokenVec &GetDiscoveryTypes() const override
  {
    static NdrTokenVec s_discoveryTypes{_tokens->rpr};
    return s_discoveryTypes;
  }

  const TfToken &GetSourceType() const override
  {
    return _tokens->rpr;
  }
};

NDR_REGISTER_PARSER_PLUGIN(HdRprNdrParserPlugin);

WABI_NAMESPACE_END
