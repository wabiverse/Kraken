set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hioOpenVDB)

if(NOT ${WITH_GPU_SUPPORT})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT is OFF")
  return()
endif()

wabi_library(hioOpenVDB
    LIBRARIES
        ar
        gf
        hio
        tf
        usd
        ${OPENEXR_Half_LIBRARY}
        ${OPENVDB_LIBRARY}

    INCLUDE_DIRS
        ${OPENVDB_INCLUDE_DIR}

    PUBLIC_CLASSES
        utils
        vdbAssetInterface

    PUBLIC_HEADERS
        api.h

    PRIVATE_CLASSES
        debugCodes
        vdbTextureData

    RESOURCE_FILES
        resources/plugInfo.json
)
