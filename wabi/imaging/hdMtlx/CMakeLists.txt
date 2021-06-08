set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hdMtlx)

if(NOT WITH_MATERIALX)
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_MATERIALX is OFF")
  return()
endif()

wabi_library(hdMtlx
  LIBRARIES
    gf
    sdf
    sdr
    tf
    trace
    vt
    ${MATERIALX_LIBRARIES}

  INCLUDE_DIRS
    ${MATERIALX_INCLUDE_DIRS}

  PUBLIC_CLASSES
    hdMtlx

  PUBLIC_HEADERS
    api.h
)