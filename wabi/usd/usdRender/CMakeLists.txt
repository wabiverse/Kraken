set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdRender)

wabi_library(usdRender
    LIBRARIES
        gf
        tf
        usd
        usdGeom

    PUBLIC_CLASSES
        product
        settings
        settingsBase
        spec
        tokens
        var

    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapProduct.cpp
        wrapSettingsBase.cpp
        wrapSettings.cpp
        wrapTokens.cpp
        wrapVar.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/generatedSchema.usda
        resources/schema.usda:usdRender/schema.usda

    DOXYGEN_FILES
        docs/overview.dox
        docs/images/aperture.svg
        docs/images/dataWindow.svg
        docs/images/nonSquarePixels.svg
        docs/images/pixelAspectRatio.svg
)
