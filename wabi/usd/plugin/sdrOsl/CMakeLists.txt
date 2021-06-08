set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE sdrOsl)

if(NOT WITH_OSL)
  return()
endif()

wabi_plugin(sdrOsl
    LIBRARIES
        gf
        tf
        vt
        ar
        ndr
        sdr
        ${OSL_QUERY_LIBRARY}
        ${OIIO_LIBRARIES}

    INCLUDE_DIRS
        ${OPENEXR_INCLUDE_DIR}
        ${OSL_INCLUDE_DIR}
        ${OIIO_INCLUDE_DIRS}

    PUBLIC_CLASSES
        oslParser

    PRIVATE_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapOslParser.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
)
