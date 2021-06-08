set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE gdr)

wabi_library(gdr
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
        registry
        geomMetadataHelpers
        geomNode
        geomProperty

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapRegistry.cpp
        wrapGeomNode.cpp
        wrapGeomProperty.cpp

    PYMODULE_FILES
        python/__init__.py

    DOXYGEN_FILES
        docs/overview.dox
)
