//
// Copyright 2017 Pixar
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
#ifndef WABI_IMAGING_PLUGIN_HD_STORM_RENDERER_PLUGIN_H
#define WABI_IMAGING_PLUGIN_HD_STORM_RENDERER_PLUGIN_H

#include "wabi/wabi.h"
#include "wabi/imaging/hd/rendererPlugin.h"

WABI_NAMESPACE_BEGIN

class HdStormRendererPlugin final : public HdRendererPlugin {
public:
    HdStormRendererPlugin()          = default;
    virtual ~HdStormRendererPlugin() = default;

    virtual HdRenderDelegate *CreateRenderDelegate() override;
    virtual HdRenderDelegate *CreateRenderDelegate(
        HdRenderSettingsMap const& settingsMap) override;

    virtual void DeleteRenderDelegate(HdRenderDelegate *renderDelegate) 
        override;

    virtual bool IsSupported() const override;

private:
    HdStormRendererPlugin(const HdStormRendererPlugin &)             = delete;
    HdStormRendererPlugin &operator =(const HdStormRendererPlugin &) = delete;
};

WABI_NAMESPACE_END

#endif // WABI_IMAGING_PLUGIN_HD_STORM_RENDERER_PLUGIN_H
