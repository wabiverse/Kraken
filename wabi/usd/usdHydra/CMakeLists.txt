set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdHydra)

wabi_library(usdHydra

    LIBRARIES
        tf
        usd
        usdShade

    PUBLIC_CLASSES
        tokens
        discoveryPlugin

    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapTokens.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/shaders/shaderDefs.usda
        resources/shaders/empty.glslfx

    DOXYGEN_FILES
        docs/overview.dox
)
