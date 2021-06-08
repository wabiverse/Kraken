set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE rprUsd)

add_definitions(-DRPR_API_USE_HEADER_V2)

wabi_library(rprUsd
    DISABLE_PRECOMPILED_HEADERS

    LIBRARIES
        gf
        tf
        vt
        hd
        glf
        sdf
        arch
        work
        ${PRORENDER_LIBRARIES}

    INCLUDE_DIRS
        ${PRORENDER_INCLUDE_DIR}
        json

    PUBLIC_CLASSES
        util
        config
        tokens
        contextHelpers
        coreImage
        debugCodes
        imageCache
        material
        materialMappings
        materialRegistry

    PUBLIC_HEADERS
        api.h
        contextMetadata.h
        error.h
        helpers.h
        materialHelpers.h
        boostIncludePath.h
        materialNodes/rpr/materialXNode.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapConfig.cpp

    PYMODULE_FILES
        python/__init__.py
)

target_sources(rprUsd PRIVATE
    materialNodes/materialNode.h
    materialNodes/usdNode.h
    materialNodes/usdNode.cpp
    materialNodes/mtlxNode.h
    materialNodes/mtlxNode.cpp
    materialNodes/rpr/baseNode.h
    materialNodes/rpr/baseNode.cpp
    materialNodes/rpr/nodeInfo.h
    materialNodes/rpr/toonNode.cpp
    materialNodes/rpr/catcherNode.cpp
    materialNodes/rpr/displaceNode.cpp
    materialNodes/rpr/materialXNode.cpp
    materialNodes/rpr/arithmeticNode.h
    materialNodes/rpr/arithmeticNode.cpp
    materialNodes/rpr/combineShadersNode.cpp
    materialNodes/houdiniPrincipledShaderNode.h
    materialNodes/houdiniPrincipledShaderNode.cpp)

add_subdirectory(materialNodes/mtlxFiles)

# GroupSources(rprUsd)

if(WITH_VULKAN)
    target_link_libraries(rprUsd ${Vulkan_LIBRARIES})
    target_include_directories(rprUsd PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_compile_definitions(rprUsd PUBLIC HDRPR_ENABLE_VULKAN_INTEROP_SUPPORT)
endif()
