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
        blitCmds.mm
        buffer.mm
        capabilities.mm
        computePipeline.mm
        computeCmds.mm
        conversions.mm
        diagnostic.mm
        graphicsCmds.mm
        graphicsPipeline.mm
        hgi.mm
        resourceBindings.mm
        sampler.mm
        shaderFunction.mm
        shaderGenerator.mm
        shaderProgram.mm
        shaderSection.mm
        texture.mm

    RESOURCE_FILES
        resources/plugInfo.json
)
