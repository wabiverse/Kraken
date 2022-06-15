set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hioOiio)

if(NOT ${WITH_GPU_SUPPORT})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT is OFF")
  return()
endif()

wabi_plugin(hioOiio
    LIBRARIES
        ar
        arch
        gf
        hio
        tf
        ${OIIO_LIBRARIES}
        ${OPENEXR_LIBRARIES}

    INCLUDE_DIRS
        ${OIIO_INCLUDE_DIRS}
        ${OPENEXR_INCLUDE_DIRS}

    CPPFILES
        oiioImage.cpp
        metadata.cpp

    RESOURCE_FILES
        resources/plugInfo.json
)
