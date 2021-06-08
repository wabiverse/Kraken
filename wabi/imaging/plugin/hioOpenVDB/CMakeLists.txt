set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hioOpenVDB)

if (NOT ${WITH_GPU_SUPPORT})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT is OFF")
  return()
endif()

wabi_plugin(hioOpenVDB
    LIBRARIES
        ar
        gf
        hio
        tf
        ${TBB_LIBRARIES}
        ${OPENEXR_LIBRARIES}
        ${OPENVDB_LIBRARY}

    INCLUDE_DIRS
        ${TBB_INCLUDE_DIRS}
        ${OPENVDB_INCLUDE_DIR}
        ${OPENEXR_INCLUDE_DIR}

    PRIVATE_CLASSES
        debugCodes
        vdbTextureData

    RESOURCE_FILES
        resources/plugInfo.json
)
