set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdRi)

wabi_library(usdRi
    LIBRARIES
        tf
        vt
        sdf
        usd
        usdShade
        usdGeom
        usdLux
        ${Boost_PYTHON_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}

    PUBLIC_CLASSES
        lightAPI
        lightFilterAPI
        lightPortalAPI
        materialAPI
        pxrAovLight
        pxrBarnLightFilter
        pxrCookieLightFilter
        pxrEnvDayLight
        pxrIntMultLightFilter
        pxrRampLightFilter
        pxrRodLightFilter
        risBxdf
        risIntegrator
        risObject
        risOslPattern
        risPattern
        rmanUtilities
        rslShader
        splineAPI
        statementsAPI
        textureAPI
        tokens

    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PRIVATE_CLASSES
        typeUtils

    PYMODULE_CPPFILES
        module.cpp
        wrapLightAPI.cpp
        wrapLightFilterAPI.cpp
        wrapLightPortalAPI.cpp
        wrapMaterialAPI.cpp
        wrapPxrAovLight.cpp
        wrapPxrBarnLightFilter.cpp
        wrapPxrCookieLightFilter.cpp
        wrapPxrEnvDayLight.cpp
        wrapPxrIntMultLightFilter.cpp
        wrapPxrRampLightFilter.cpp
        wrapPxrRodLightFilter.cpp
        wrapRisBxdf.cpp
        wrapRisIntegrator.cpp
        wrapRisObject.cpp
        wrapRisOslPattern.cpp
        wrapRisPattern.cpp
        wrapRmanUtilities.cpp
        wrapRslShader.cpp
        wrapSplineAPI.cpp
        wrapStatementsAPI.cpp
        wrapTextureAPI.cpp
        wrapTokens.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/generatedSchema.usda
        resources/schema.usda:usdRi/schema.usda

    DOXYGEN_FILES
        docs/overview.dox
)
