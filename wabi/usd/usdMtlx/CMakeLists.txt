set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdMtlx)

if (MATERIALX_STDLIB_DIR)
    add_definitions(-DWABI_MATERIALX_STDLIB_DIR="${MATERIALX_STDLIB_DIR}")
endif()

wabi_library(usdMtlx
    LIBRARIES
        arch
        gf
        ndr
        sdf
        sdr
        tf
        vt
        usd
        usdGeom
        usdShade
        usdUI
        usdUtils
        ${MATERIALX_LIBRARIES}

    INCLUDE_DIRS
        ${MATERIALX_INCLUDE_DIRS}

    CPPFILES
        debugCodes.cpp
        discovery.cpp
        parser.cpp

    PRIVATE_CLASSES
        backdoor
        fileFormat

    PUBLIC_CLASSES
        reader
        utils

    PRIVATE_HEADERS
        debugCodes.h

    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapBackdoor.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)
