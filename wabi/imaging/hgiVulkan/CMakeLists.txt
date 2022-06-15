set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hgiVulkan)

if(NOT ${WITH_GPU_SUPPORT} OR NOT ${WITH_VULKAN})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT or WITH_VULKAN is OFF")
  return()
endif()

wabi_library(hgiVulkan
    LIBRARIES
        arch
        hgi
        tf
        trace
        ${VULKAN_LIBS}

    PUBLIC_CLASSES
        blitCmds
        buffer
        capabilities
        commandBuffer
        commandQueue
        computeCmds
        computePipeline
        conversions
        device
        diagnostic
        garbageCollector
        graphicsCmds
        graphicsPipeline
        hgi
        instance
        pipelineCache
        resourceBindings
        sampler
        shaderCompiler
        shaderFunction
        shaderGenerator
        shaderProgram
        shaderSection
        texture

    PUBLIC_HEADERS
        api.h
        vulkan.h
        vk_mem_alloc.h

    PRIVATE_CLASSES
        spirv_reflect

    RESOURCE_FILES
        resources/plugInfo.json
)
