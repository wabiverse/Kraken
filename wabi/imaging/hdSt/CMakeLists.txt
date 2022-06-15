set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hdSt)

set(optionalLibs "")
set(optionalIncludeDirs "")
set(optionalPublicClasses "")
set(optionalPrivateClasses "")

if (WITH_MATERIALX)
    list(APPEND optionalLibs ${MATERIALX_LIBRARIES} hdMtlx)
    list(APPEND optionalIncludeDirs ${MATERIALX_INCLUDE_DIRS})
    list(APPEND optionalPrivateClasses materialXFilter materialXShaderGen)
endif()

if (WITH_PTEX)
    list(APPEND optionalLibs ${PTEX_LIBRARY})
    list(APPEND optionalIncludeDirs ${PTEX_INCLUDE_DIR})
    list(APPEND optionalPublicClasses ptexMipmapTextureLoader)
endif()

wabi_library(hdSt
    LIBRARIES
        hio
        garch
        glf
        hd
        hgiGL
        hgiInterop
        sdr
        tf
        trace
        ${OPENSUBDIV_LIBRARIES}
        ${optionalLibs}

    INCLUDE_DIRS
        ${OPENSUBDIV_INCLUDE_DIR}
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
        bufferUtils
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
        stagingBuffer
        subtextureIdentifier
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
        drawItemsCache
        extCompComputeShader
        fieldTextureCpuData
        imageShaderShaderKey
        indirectDrawBatch
        materialNetworkShader
        materialParam
        meshShaderKey
        meshTopology 
        pipelineDrawBatch
        pointsShaderKey
        quadrangulate
        resourceBinder
        resourceLayout
        shaderKey
        subdivision
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
        resources/shaders/fallbackMaterialNetwork.glslfx
        resources/shaders/fallbackVolume.glslfx
        resources/shaders/frustumCull.glslfx
        resources/shaders/imageShader.glslfx
        resources/shaders/instancing.glslfx
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

