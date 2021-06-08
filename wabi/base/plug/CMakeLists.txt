set(WABI_PREFIX wabi/base)
set(WABI_PACKAGE plug)

wabi_library(plug
    LIBRARIES
        arch
        tf
        js
        trace
        work
        ${Boost_PYTHON_LIBRARY}
        ${TBB_LIBRARIES}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        interfaceFactory
        notice
        plugin
        registry
        staticInterface

    PRIVATE_CLASSES
        debugCodes
        info
        testPlugBase

    PUBLIC_HEADERS
        api.h
        thisPlugin.h

    CPPFILES
        initConfig.cpp

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapNotice.cpp
        wrapPlugin.cpp
        wrapRegistry.cpp
        wrapTestPlugBase.cpp

    PYMODULE_FILES
        python/__init__.py

    DOXYGEN_FILES
        docs/overview.dox
)
