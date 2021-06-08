set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdMedia)

wabi_library(usdMedia
    LIBRARIES
        tf
        vt
        sdf
        usd
        usdGeom

    PUBLIC_CLASSES
        spatialAudio
        tokens

    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapSpatialAudio.cpp
        wrapTokens.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/generatedSchema.usda
        resources/schema.usda:usdMedia/schema.usda

    DOXYGEN_FILES
        docs/overview.dox
)
