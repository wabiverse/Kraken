set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE sdr)

wabi_library(sdr
    LIBRARIES
        tf
        vt
        ar
        ndr
        sdf
        ${Boost_PYTHON_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}

    PUBLIC_HEADERS
        api.h
        declare.h

    PUBLIC_CLASSES
        debugCodes
        registry
        shaderMetadataHelpers
        shaderNode
        shaderProperty

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapRegistry.cpp
        wrapShaderNode.cpp
        wrapShaderProperty.cpp

    PYMODULE_FILES
        python/__init__.py
        python/shaderParserTestUtils.py

    DOXYGEN_FILES
        docs/overview.dox
)

