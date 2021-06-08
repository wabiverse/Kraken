set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE ndrArnold)

if(NOT ARNOLD_FOUND)
  message(WARNING "Not building ${WABI_PACKAGE} because of missing dependency: Arnold")
  return()
endif()

wabi_plugin(ndrArnold
    LIBRARIES
        tf
        gf
        vt
        ndr
        sdr
        sdf
        usd
        ${ARNOLD_LIBRARIES}

    INCLUDE_DIRS
        ${ARNOLD_INCLUDE_DIRS}

    PRIVATE_CLASSES
        discovery
        parser
        utils

    PRIVATE_HEADERS
        api.h
        ndrarnold.h

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
)
    