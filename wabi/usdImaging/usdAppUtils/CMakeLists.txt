set(WABI_PREFIX wabi/usdImaging)
set(WABI_PACKAGE usdAppUtils)

if(NOT ${WITH_GPU_SUPPORT})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT is OFF")
  return()
endif()

wabi_library(usdAppUtils
    LIBRARIES
        garch
        gf
        hio
        sdf
        tf
        usd
        usdGeom
        usdImagingGL
        ${Boost_PYTHON_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}

    PUBLIC_HEADERS
        api.h

    PUBLIC_CLASSES
        camera
        frameRecorder

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapCamera.cpp
        wrapFrameRecorder.cpp

    PYMODULE_FILES
        python/__init__.py
        python/cameraArgs.py
        python/colorArgs.py
        python/complexityArgs.py
        python/framesArgs.py
        python/rendererArgs.py

    DOXYGEN_FILES
        docs/overview.dox
)
