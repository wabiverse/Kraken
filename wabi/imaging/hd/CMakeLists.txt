set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hd)

set(optionalPublicClasses "")

wabi_library(hd
    LIBRARIES
        plug
        tf
        trace
        vt
        work
        sdf
        cameraUtil
        hf
        pxOsd
        ${TBB_LIBRARIES}

    INCLUDE_DIRS
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        aov
        basisCurves
        basisCurvesTopology
        binding
        bprim
        bufferArray
        bufferArrayRange
        bufferArrayRegistry
        bufferResource
        bufferSource
        bufferSpec
        camera
        changeTracker
        command
        compExtCompInputSource
        computation
        coordSys
        debugCodes
        dirtyList
        drawingCoord
        drawItem
        engine
        enums
        extCompCpuComputation
        extCompInputSource
        extCompPrimvarBufferSource
        extComputation
        extComputationContext
        extComputationUtils
        field
        flatNormals
        geomSubset
        instancer
        instanceRegistry
        light
        material
        mesh
        meshTopology
        meshUtil
        perfLog
        pluginRenderDelegateUniqueHandle
        points
        primGather
        primTypeIndex
        renderBuffer
        renderDelegate
        renderIndex
        renderPass
        renderPassState
        rendererPlugin
        rendererPluginHandle
        rendererPluginRegistry
        renderThread
        repr
        resource
        resourceRegistry
        rprim
        rprimCollection
        rprimSharedData
        sceneDelegate
        sceneExtCompInputSource
        selection
        smoothNormals
        sortedIds                   
        sprim
        strategyBase
        task
        timeSampleArray
        tokens
        topology
        types
        unitTestDelegate
        unitTestHelper
        unitTestNullRenderDelegate
        unitTestNullRenderPass
        vertexAdjacency
        volume
        vtBufferSource

    PUBLIC_HEADERS
        api.h
        driver.h
        version.h

    PRIVATE_CLASSES
        extComputationContextInternal
        ${optionalPrivateClasses}

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)
