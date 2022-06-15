set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdVol)

wabi_library(usdVol

    LIBRARIES
        tf
        usd
        usdGeom

    PUBLIC_CLASSES
        volume
        fieldBase
        fieldAsset
        field3DAsset
        openVDBAsset
        tokens

    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapVolume.cpp
        wrapFieldBase.cpp
        wrapFieldAsset.cpp
        wrapField3DAsset.cpp
        wrapOpenVDBAsset.cpp
        wrapTokens.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/generatedSchema.usda
        resources/plugInfo.json
        resources/schema.usda:usdVol/schema.usda

    DOXYGEN_FILES
        overview.dox
)
