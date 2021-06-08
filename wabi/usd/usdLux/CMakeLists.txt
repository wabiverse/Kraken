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
        cylinderLight
        diskLight
        distantLight
        domeLight
        geometryLight
        light
        lightFilter
        lightPortal
        listAPI
        pluginLight
        pluginLightFilter
        portalLight
        rectLight
        shadowAPI
        shapingAPI
        sphereLight
        tokens

    PUBLIC_HEADERS
        api.h

    PRIVATE_CLASSES
        discoveryPlugin
        lightDefParser

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapCylinderLight.cpp
        wrapDiskLight.cpp
        wrapDistantLight.cpp
        wrapDomeLight.cpp
        wrapGeometryLight.cpp
        wrapLight.cpp
        wrapLightFilter.cpp
        wrapLightPortal.cpp
        wrapListAPI.cpp
        wrapPluginLight.cpp
        wrapPluginLightFilter.cpp
        wrapPortalLight.cpp
        wrapRectLight.cpp
        wrapShadowAPI.cpp
        wrapShapingAPI.cpp
        wrapSphereLight.cpp
        wrapTokens.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/generatedSchema.usda
        resources/schema.usda:usdLux/schema.usda

    DOXYGEN_FILES
        docs/overview.dox
)
