set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdUI)

wabi_library(usdUI
    LIBRARIES
        tf
        vt
        sdf
        usd

    PUBLIC_CLASSES
        area
        backdrop
        nodeGraphNodeAPI
        sceneGraphPrimAPI
        screen
        tokens
        userPref
        window
        workspace

    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapArea.cpp
        wrapBackdrop.cpp
        wrapNodeGraphNodeAPI.cpp
        wrapSceneGraphPrimAPI.cpp
        wrapScreen.cpp
        wrapTokens.cpp
        wrapUserPref.cpp
        wrapWindow.cpp
        wrapWorkspace.cpp
    
    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/generatedSchema.usda
        resources/schema.usda:usdUI/schema.usda

    DOXYGEN_FILES
        docs/overview.dox
)
