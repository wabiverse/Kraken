set(WABI_PREFIX wabi/usdImaging)
set(WABI_PACKAGE usdApollo)

if(NOT ${WITH_GPU_SUPPORT})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT is OFF")
  return()
endif()

# Note: both python include directives are included for compatibility.
wabi_library(usdApollo
  LIBRARIES
    gf
    tf
    plug
    trace
    vt
    work
    hio
    garch
    glf
    hd
    hdx
    pxOsd
    sdf
    sdr
    usd
    usdGeom
    usdHydra
    usdShade
    usdImaging
    ar
    ${Boost_PYTHON_LIBRARY}
    ${PYTHON_LIBRARIES}
    ${TBB_LIBRARIES}

  INCLUDE_DIRS
    ${PYTHON_INCLUDE_PATH}
    ${PYTHON_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}

  PUBLIC_CLASSES
    drawModeAdapter
    engine
    package
    renderParams

  PUBLIC_HEADERS
    api.h
    rendererSettings.h
    version.h

  RESOURCE_FILES
    resources/plugInfo.json
    resources/shaders/drawMode.glslfx

  DISABLE_PRECOMPILED_HEADERS
)
