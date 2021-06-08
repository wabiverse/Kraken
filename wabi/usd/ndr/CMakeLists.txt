set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE ndr)

wabi_library(ndr
    LIBRARIES
        tf
        plug
        vt
        work
        ar
        sdf
        ${Boost_PYTHON_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}

    PUBLIC_CLASSES
        debugCodes
        declare
        discoveryPlugin
        filesystemDiscovery
        filesystemDiscoveryHelpers
        node
        parserPlugin
        property
        registry

    PUBLIC_HEADERS
        api.h
        nodeDiscoveryResult.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapDeclare.cpp
        wrapDiscoveryPlugin.cpp
        wrapFilesystemDiscovery.cpp
        wrapFilesystemDiscoveryHelpers.cpp
        wrapNode.cpp
        wrapNodeDiscoveryResult.cpp
        wrapProperty.cpp
        wrapRegistry.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)
