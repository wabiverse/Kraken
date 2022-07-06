# 
#  Copyright 2021 Pixar. All Rights Reserved.
# 
#  Portions of this file are derived from original work by Pixar
#  distributed with Universal Scene Description, a project of the
#  Academy Software Foundation (ASWF). https://www.aswf.io/
# 
#  Licensed under the Apache License, Version 2.0 (the "Apache License")
#  with the following modification you may not use this file except in
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
kpy -- The Kraken Python Module.
"""

__all__ = (
    "app",
    "context",
    "data",
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
    "Usdviewq",
#     "ops",
#     "path",
#     "props",
    "types",
    "utils",
)


# internal kraken C module
from _kpy import (
    app,
    context,
    data,
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
    Usdviewq,
#     msgbus,
#     props,
    types,
)

# python modules
from . import (
#     ops,
#     path,
    utils,
)


def main():
    import sys

    # Possibly temp. addons path
    from os.path import join, dirname
    sys.path.extend([
        join(dirname(dirname(dirname(__file__))), "addons", "modules"),
        join(utils.user_resource('SCRIPTS'), "addons", "modules"),
    ])

    # fake module to allow:
    #   from kpy.types import Panel
    sys.modules.update({
        "kpy.app": app,
        "kpy.Tf": Tf, 
        "kpy.Gf": Gf,
        "kpy.Trace": Trace,
        "kpy.Work": Work,
        "kpy.Plug": Plug,
        "kpy.Vt": Vt,
        "kpy.Ar": Ar,
        "kpy.Kind": Kind,
        "kpy.Sdf": Sdf,
        "kpy.Ndr": Ndr,
        "kpy.Sdr": Sdr,
        "kpy.Pcp": Pcp,
        "kpy.Usd": Usd,
        "kpy.UsdGeom": UsdGeom,
        "kpy.UsdVol": UsdVol,
        "kpy.UsdMedia": UsdMedia,
        "kpy.UsdShade": UsdShade,
        "kpy.UsdLux": UsdLux,
        "kpy.UsdRender": UsdRender,
        "kpy.UsdHydra": UsdHydra,
        "kpy.UsdRi": UsdRi,
        "kpy.UsdSkel": UsdSkel,
        "kpy.UsdUI": UsdUI,
        "kpy.UsdUtils": UsdUtils,
        "kpy.UsdPhysics": UsdPhysics,
        "kpy.UsdAbc": UsdAbc,
        "kpy.UsdDraco": UsdDraco,
        "kpy.Garch": Garch,
        "kpy.CameraUtil": CameraUtil,
        "kpy.PxOsd": PxOsd,
        "kpy.Glf": Glf,
        "kpy.UsdImagingGL": UsdImagingGL,
        "kpy.UsdAppUtils": UsdAppUtils,
        "kpy.Usdviewq": Usdviewq,
        # "kpy.app.handlers": app.handlers,
        # "kpy.app.translations": app.translations,
        "kpy.types": types,
    })

    # Initializes Python classes.
    # (good place to run a profiler or trace).
    utils.load_scripts()


main()

del main
