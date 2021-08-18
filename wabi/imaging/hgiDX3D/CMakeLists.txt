set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hgiDX3D)

if(NOT ${WITH_GPU_SUPPORT} OR NOT ${WITH_DIRECTX})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT or WITH_DIRECTX is OFF")
  return()
endif()

wabi_library(hgiDX3D
    LIBRARIES
        arch
        hgi
        tf
        trace
        ${DIRECTX_LIBS}

    PUBLIC_CLASSES
        hgi

    PUBLIC_HEADERS
        api.h
        dx3d.h

    RESOURCE_FILES
        resources/plugInfo.json
)
