set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdUI)

wabi_library(usdUI
    LIBRARIES
        tf
        vt
        sdf
        usd

    PUBLIC_CLASSES
        backdrop
        nodeGraphNodeAPI
        sceneGraphPrimAPI
        tokens

    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapBackdrop.cpp
        wrapNodeGraphNodeAPI.cpp
        wrapSceneGraphPrimAPI.cpp
        wrapTokens.cpp
    
    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/generatedSchema.usda
        resources/schema.usda:usdUI/schema.usda

    DOXYGEN_FILES
        docs/overview.dox
)
