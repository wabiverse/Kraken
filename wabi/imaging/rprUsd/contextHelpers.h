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

#ifndef WABI_IMAGING_RPR_USD_CONTEXT_HELPERS_H
#define WABI_IMAGING_RPR_USD_CONTEXT_HELPERS_H

#include "wabi/imaging/rprUsd/api.h"

namespace rpr
{
  class Context;
}

WABI_NAMESPACE_BEGIN

struct RprUsdContextMetadata;

RPRUSD_API
rpr::Context *RprUsdCreateContext(RprUsdContextMetadata *metadata);

RPRUSD_API
bool RprUsdIsTracingEnabled();

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_RPR_USD_CONTEXT_HELPERS_H
