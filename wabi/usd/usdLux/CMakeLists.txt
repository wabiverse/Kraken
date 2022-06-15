set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdLux)

wabi_library(usdLux
    LIBRARIES
        tf
        vt
        ndr
        sdf
        usd
        usdGeom
        usdShade

    PUBLIC_CLASSES
        blackbody
        boundableLightBase
        cylinderLight
        diskLight
        distantLight
        domeLight
        geometryLight
        lightAPI
        lightFilter
        lightListAPI
        listAPI
        meshLightAPI
        nonboundableLightBase
        pluginLight
        pluginLightFilter
        portalLight
        rectLight
        shadowAPI
        shapingAPI
        sphereLight
        tokens
        volumeLightAPI

    PUBLIC_HEADERS
        api.h

    PRIVATE_CLASSES
        discoveryPlugin
        lightDefParser

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapBoundableLightBase.cpp
        wrapCylinderLight.cpp
        wrapDiskLight.cpp
        wrapDistantLight.cpp
        wrapDomeLight.cpp
        wrapGeometryLight.cpp
        wrapLightAPI.cpp
        wrapLightFilter.cpp
        wrapLightListAPI.cpp
        wrapListAPI.cpp
        wrapMeshLightAPI.cpp
        wrapNonboundableLightBase.cpp
        wrapPluginLight.cpp
        wrapPluginLightFilter.cpp
        wrapPortalLight.cpp
        wrapRectLight.cpp
        wrapShadowAPI.cpp
        wrapShapingAPI.cpp
        wrapSphereLight.cpp
        wrapTokens.cpp
        wrapVolumeLightAPI.cpp

    PYMODULE_FILES 
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/generatedSchema.usda
        resources/schema.usda:usdLux/schema.usda

    DOXYGEN_FILES
        docs/overview.dox
)
