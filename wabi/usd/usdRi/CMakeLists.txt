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
        ${Boost_PYTHON_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}

    PUBLIC_CLASSES
        materialAPI
        rmanUtilities
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
        wrapMaterialAPI.cpp
        wrapRmanUtilities.cpp
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
