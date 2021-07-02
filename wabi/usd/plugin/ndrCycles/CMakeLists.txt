set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE ndrCycles)

if(NOT CYCLES_FOUND)
  message(WARNING "Not building ${WABI_PACKAGE} because of missing dependency: Cycles")
  return()
endif()

wabi_plugin(ndrCycles
    LIBRARIES
        tf
        gf
        vt
        ndr
        sdr
        sdf
        usd
        ${EMBREE_LIBRARY}
        ${CYCLES_LIBRARIES}

    INCLUDE_DIRS
        ${EMBREE_INCLUDE_DIR}
        ${CYCLES_INCLUDE_DIRS}

    PRIVATE_CLASSES
        discovery
        parser

    PRIVATE_HEADERS
        api.h

    RESOURCE_FILES
        resources/plugInfo.json

    DISABLE_PRECOMPILED_HEADERS
)
    