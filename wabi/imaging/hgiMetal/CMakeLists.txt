set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hgiMetal)

if(NOT ${WITH_GPU_SUPPORT} OR NOT ${WITH_METAL})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT or WITH_METAL is OFF")
  return()
endif()

FIND_LIBRARY(METAL_LIBRARY Metal)
FIND_LIBRARY(APPUIKIT_LIBRARY AppKit)

wabi_library(hgiMetal
    LIBRARIES
        arch
        hgi
        tf
        trace
        ${METAL_LIBRARY}
        ${APPUIKIT_LIBRARY}
        "-framework Foundation"
        "-framework Metal"
        "-framework QuartzCore"

    INCLUDE_DIRS
        ${APPLE_CXX_INCLUDE_DIR}

    PUBLIC_HEADERS
        api.h
        capabilities.h
        computeCmds.h
        computePipeline.h
        diagnostic.h
        hgi.h
        texture.h
        blitCmds.h
        buffer.h
        graphicsCmds.h
        graphicsPipeline.h
        resourceBindings.h
        sampler.h
        shaderFunction.h
        shaderProgram.h

    PRIVATE_HEADERS
        conversions.h
        shaderGenerator.h
        shaderSection.h

    CPPFILES
        blitCmds.cpp
        buffer.cpp
        capabilities.cpp
        computePipeline.cpp
        computeCmds.cpp
        conversions.cpp
        diagnostic.cpp
        graphicsCmds.cpp
        graphicsPipeline.cpp
        hgi.cpp
        metal.cpp
        resourceBindings.cpp
        sampler.cpp
        shaderFunction.cpp
        shaderGenerator.cpp
        shaderProgram.cpp
        shaderSection.cpp
        texture.cpp

    RESOURCE_FILES
        resources/plugInfo.json
)
