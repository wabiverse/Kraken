set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE vkf)

if(NOT ${WITH_VULKAN})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_VULKAN is OFF")
  return()
endif()

wabi_library(vkf
  LIBRARIES
    ar
    arch
    garch
    gf
    hf
    hio
    js
    plug
    tf
    trace
    sdf
    ${Boost_PYTHON_LIBRARY}
    ${VULKAN_LIBRARIES}
    ${X11_LIBRARIES}

  INCLUDE_DIRS
    ${Boost_INCLUDE_DIRS}
    ${VULKAN_INCLUDE_DIRS}

  PUBLIC_CLASSES
    deviceCaps

  PRIVATE_CLASSES
    debugCodes

  PUBLIC_HEADERS
    api.h
    
  DISABLE_PRECOMPILED_HEADERS
)
