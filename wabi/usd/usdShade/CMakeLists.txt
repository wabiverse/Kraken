set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdShade)

wabi_library(usdShade
    LIBRARIES
        tf
        vt
        sdf
        ndr
        sdr
        usd
        usdGeom

    PUBLIC_CLASSES
        connectableAPI
        connectableAPIBehavior
        coordSysAPI
        input
        material
        materialBindingAPI
        nodeDefAPI
        output
        shader
        shaderDefParser
        shaderDefUtils
        nodeGraph
        tokens
        utils

    PUBLIC_HEADERS
        api.h
        types.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapConnectableAPI.cpp
        wrapCoordSysAPI.cpp
        wrapInput.cpp
        wrapMaterial.cpp
        wrapMaterialBindingAPI.cpp
        wrapNodeDefAPI.cpp
        wrapNodeGraph.cpp
        wrapOutput.cpp
        wrapShader.cpp
        wrapShaderDefParser.cpp
        wrapShaderDefUtils.cpp
        wrapTokens.cpp
        wrapTypes.cpp
        wrapUtils.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/generatedSchema.usda
        resources/schema.usda:usdShade/schema.usda

    DOXYGEN_FILES
        docs/overview.dox
)
