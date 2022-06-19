# 
#  Copyright 2021 Pixar. All Rights Reserved.
# 
#  Portions of this file are derived from original work by Pixar
#  distributed with Universal Scene Description, a project of the
#  Academy Software Foundation (ASWF). https://www.aswf.io/
# 
#  Licensed under the Apache License, Version 2.0 (the "Apache License")
#  with the following modification; you may not use this file except in
#  compliance with the Apache License and the following modification:
#  Section 6. Trademarks. is deleted and replaced with:
# 
#  6. Trademarks. This License does not grant permission to use the trade
#     names, trademarks, service marks, or product names of the Licensor
#     and its affiliates, except as required to comply with Section 4(c)
#     of the License and to reproduce the content of the NOTICE file.
#
#  You may obtain a copy of the Apache License at:
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the Apache License with the above modification is
#  distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
#  ANY KIND, either express or implied. See the Apache License for the
#  specific language governing permissions and limitations under the
#  Apache License.
#
#  Modifications copyright (C) 2020-2021 Wabi.
#
"""
wabi -- The Pixar Python Module.
"""

__all__ = (
    "Tf", 
    "Gf", 
    "Trace", 
    "Work", 
    "Plug", 
    "Vt", 
    "Ar", 
    "Kind", 
    "Sdf", 
    "Ndr", 
    "Sdr", 
    "Pcp", 
    "Usd", 
    "UsdGeom", 
    "UsdVol", 
    "UsdMedia", 
    "UsdShade", 
    "UsdLux", 
    "UsdRender", 
    "UsdHydra", 
    "UsdRi", 
    "UsdSkel", 
    "UsdUI", 
    "UsdUtils", 
    "UsdPhysics", 
    "UsdAbc", 
    "UsdDraco", 
    "Garch", 
    "CameraUtil", 
    "PxOsd", 
    "Glf", 
    "UsdImagingGL", 
    "UsdAppUtils", 
    "Usdviewq"
)


# internal wabi CXX module
from _wabi import (
    Tf, 
    Gf, 
    Trace,
    Work, 
    Plug, 
    Vt, 
    Ar, 
    Kind, 
    Sdf, 
    Ndr, 
    Sdr, 
    Pcp, 
    Usd, 
    UsdGeom, 
    UsdVol, 
    UsdMedia, 
    UsdShade, 
    UsdLux, 
    UsdRender, 
    UsdHydra, 
    UsdRi, 
    UsdSkel, 
    UsdUI, 
    UsdUtils, 
    UsdPhysics, 
    UsdAbc, 
    UsdDraco, 
    Garch, 
    CameraUtil, 
    PxOsd, 
    Glf, 
    UsdImagingGL, 
    UsdAppUtils, 
    Usdviewq
)


def main():
    import sys

    # fake module to allow:
    #   from wabi.Usd import Usd
    sys.modules.update({
        "wabi.Tf": Tf, 
        "wabi.Gf": Gf,
        "wabi.Trace": Trace,
        "wabi.Work": Work,
        "wabi.Plug": Plug,
        "wabi.Vt": Vt,
        "wabi.Ar": Ar,
        "wabi.Kind": Kind,
        "wabi.Sdf": Sdf,
        "wabi.Ndr": Ndr,
        "wabi.Sdr": Sdr,
        "wabi.Pcp": Pcp,
        "wabi.Usd": Usd,
        "wabi.UsdGeom": UsdGeom,
        "wabi.UsdVol": UsdVol,
        "wabi.UsdMedia": UsdMedia,
        "wabi.UsdShade": UsdShade,
        "wabi.UsdLux": UsdLux,
        "wabi.UsdRender": UsdRender,
        "wabi.UsdHydra": UsdHydra,
        "wabi.UsdRi": UsdRi,
        "wabi.UsdSkel": UsdSkel,
        "wabi.UsdUI": UsdUI,
        "wabi.UsdUtils": UsdUtils,
        "wabi.UsdPhysics": UsdPhysics,
        "wabi.UsdAbc": UsdAbc,
        "wabi.UsdDraco": UsdDraco,
        "wabi.Garch": Garch,
        "wabi.CameraUtil": CameraUtil,
        "wabi.PxOsd": PxOsd,
        "wabi.Glf": Glf,
        "wabi.UsdImagingGL": UsdImagingGL,
        "wabi.UsdAppUtils": UsdAppUtils,
        "wabi.Usdviewq": Usdviewq
    })


main()

del main
