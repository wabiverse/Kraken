set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hgiGL)

if (NOT ${WITH_OPENGL})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_OPENGL is OFF")
  return()
endif()

wabi_library(hgiGL
    LIBRARIES
        arch
        garch
        hgi
        tf
        trace

    PRIVATE_CLASSES
        conversions
        diagnostic
        scopedStateHolder
        shaderGenerator
        shaderSection

    PUBLIC_CLASSES
        blitCmds
        buffer
        capabilities
        computeCmds
        computePipeline
        contextArena
        debugCodes
        device
        garbageCollector
        graphicsCmds
        graphicsPipeline
        hgi
        ops
        resourceBindings
        sampler
        shaderFunction
        shaderProgram
        texture

    PUBLIC_HEADERS
        api.h

    RESOURCE_FILES
        resources/plugInfo.json
)
