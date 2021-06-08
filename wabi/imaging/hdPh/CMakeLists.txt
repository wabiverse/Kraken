set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hdPh)

# XXX Ensure Phoenix uses only Hgi, for cross platform
#     compatibility with Vulkan & Metal & OpenGL.
if(NOT ${WITH_GPU_SUPPORT})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT is OFF")
  return()
endif()

if (OPENSUBDIV_HAS_GLSL_TRANSFORM_FEEDBACK)
    add_definitions(-DOPENSUBDIV_HAS_GLSL_TRANSFORM_FEEDBACK)
endif()
if (OPENSUBDIV_HAS_GLSL_COMPUTE)
    add_definitions(-DOPENSUBDIV_HAS_GLSL_COMPUTE)
endif()

set(optionalLibs "")
set(optionalIncludeDirs "")
set(optionalPublicClasses "")
set(optionalPrivateClasses "")
if (${WITH_MATERIALX})
    add_definitions(-DWABI_MATERIALX_STDLIB_DIR="${MATERIALX_STDLIB_DIR}")
    add_definitions(-DWABI_MATERIALX_RESOURCE_DIR="${MATERIALX_RESOURCE_DIR}")

    list(APPEND optionalLibs ${MATERIALX_LIBRARIES} hdMtlx)
    list(APPEND optionalIncludeDirs ${MATERIALX_INCLUDE_DIRS})
    list(APPEND optionalPrivateClasses
         materialXFilter
         materialXShaderGen
    )
endif()
if (WITH_PTEX)
    list(APPEND optionalLibs ${PTEX_LIBRARY})
    list(APPEND optionalIncludeDirs ${PTEX_INCLUDE_DIR})
    list(APPEND optionalPublicClasses ptexMipmapTextureLoader)
endif()

wabi_library(hdPh
    LIBRARIES
        hio
        garch
        vkf
        glf
        hd
        hgiGL
        gdr
        sdr
        tf
        trace
        ${OPENSUBDIV_LIBRARIES}
        ${optionalLibs}

    INCLUDE_DIRS
        ${OPENSUBDIV_INCLUDE_DIRS}
        ${optionalIncludeDirs}

    PUBLIC_CLASSES
        basisCurves
        bufferArrayRange
        bufferResource
        commandBuffer
        copyComputation
        debugCodes
        dispatchBuffer
        domeLightComputations
        drawItem
        drawItemInstance
        drawTarget
        drawTargetRenderPassState
        dynamicUvTextureObject
        extComputation
        extCompGpuComputation
        extCompGpuComputationBufferSource
        extCompGpuComputationResource
        extCompGpuPrimvarBufferSource
        fallbackLightingShader
        field
        fieldSubtextureIdentifier
        flatNormals
        geometricShader
        glConversions
        glslfxShader
        glslProgram
        glUtils
        hgiConversions
        hioConversions
        imageShaderRenderPass
        instancer
        interleavedMemoryManager
        light
        lightingShader
        material
        materialNetwork
        mesh
        mixinShader
        package
        points
        primUtils
        ptexTextureObject
        renderBuffer
        renderDelegate
        renderParam
        renderPass
        renderPassShader
        renderPassState
        resourceRegistry
        samplerObject
        samplerObjectRegistry
        shaderCode
        simpleLightingShader
        smoothNormals
        subtextureIdentifier
        surfaceShader
        textureBinder
        textureCpuData
        textureHandle
        textureHandleRegistry
        textureIdentifier
        textureObject
        textureObjectRegistry
        textureUtils
        tokens
        udimTextureObject
        unitTestGLDrawing
        vboMemoryManager
        vboSimpleMemoryManager
        volume
        ${optionalPublicClasses}

    PUBLIC_HEADERS
        api.h
        dynamicUvTextureImplementation.h

    PRIVATE_CLASSES
        assetUvTextureCpuData
        basisCurvesComputations
        basisCurvesShaderKey
        basisCurvesTopology
        codeGen
        cullingShaderKey
        drawBatch
        extCompComputeShader
        fieldTextureCpuData
        imageShaderShaderKey
        immediateDrawBatch
        indirectDrawBatch
        materialParam
        meshShaderKey
        meshTopology
        pointsShaderKey
        quadrangulate
        resourceBinder
        shaderKey
        subdivision
        subdivision3
        triangulate
        unitTestHelper
        volumeShader
        volumeShaderKey
        ${optionalPrivateClasses}

    RESOURCE_FILES
        resources/plugInfo.json
        resources/shaders/basisCurves.glslfx
        resources/shaders/compute.glslfx
        resources/shaders/domeLight.glslfx
        resources/shaders/edgeId.glslfx
        resources/shaders/fallbackLighting.glslfx
        resources/shaders/fallbackLightingShader.glslfx
        resources/shaders/fallbackSurface.glslfx
        resources/shaders/fallbackVolume.glslfx
        resources/shaders/frustumCull.glslfx
        resources/shaders/imageShader.glslfx
        resources/shaders/instancing.glslfx
        resources/shaders/lighting.glslfx
        resources/shaders/lightingIntegrationShader.glslfx
        resources/shaders/mesh.glslfx
        resources/shaders/meshFaceCull.glslfx
        resources/shaders/meshNormal.glslfx
        resources/shaders/meshWire.glslfx
        resources/shaders/points.glslfx
        resources/shaders/pointId.glslfx
        resources/shaders/ptexTexture.glslfx
        resources/shaders/renderPass.glslfx
        resources/shaders/renderPassShader.glslfx
        resources/shaders/simpleLightingShader.glslfx
        resources/shaders/terminals.glslfx
        resources/shaders/visibility.glslfx
        resources/shaders/volume.glslfx

    DOXYGEN_FILES
        docs/overview.dox
)
