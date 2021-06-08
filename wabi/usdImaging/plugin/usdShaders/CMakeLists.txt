set(WABI_PREFIX wabi/usdImaging)
set(WABI_PACKAGE usdShaders)

wabi_plugin(usdShaders
    LIBRARIES
        ar
        ndr
        sdr
        usdShade

    PRIVATE_CLASSES
        discoveryPlugin

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/shaders/shaderDefs.usda
        resources/shaders/previewSurface.glslfx
        resources/shaders/uvTexture.glslfx
        resources/shaders/primvarReader.glslfx
        resources/shaders/transform2d.glslfx

    DOXYGEN_FILES
        docs/overview.dox
)
