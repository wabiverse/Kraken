set(WABI_PREFIX wabi/usdImaging)
set(WABI_PACKAGE sdrGlslfx)

wabi_plugin(sdrGlslfx
    LIBRARIES
        ar
        ndr
        sdr
        hio

    PRIVATE_CLASSES
        parserPlugin

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)
