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
        ${TBB_tbb_LIBRARY}

    INCLUDE_DIRS
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        aov
        basisCurves
        # basisCurvesSchema
        basisCurvesTopology
        # basisCurvesTopologySchema
        binding
        bprim
        bufferArray
        bufferArrayRange
        bufferArrayRegistry
        bufferResource
        bufferSource
        bufferSpec
        camera
        # cameraSchema
        # categoriesSchema
        changeTracker
        command
        compExtCompInputSource
        computation
        # containerDataSourceEditor
        coordSys
        # coordSysBindingSchema
        dataSource
        dataSourceLegacyPrim
        dataSourceLocator
        dataSourceMaterialNetworkInterface
        debugCodes
        # dependenciesSchema
        # dependencySchema
        # dirtyBitsTranslator
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
        # extComputationInputComputationSchema
        # extComputationOutputSchema
        # extComputationPrimvarSchema
        # extComputationPrimvarsSchema
        # extComputationSchema
        extComputationUtils
        # extentSchema
        field
        # filteringSceneIndex
        # flatteningSceneIndex
        flatNormals
        geomSubset
        # geomSubsetSchema
        # geomSubsetsSchema
        # instanceCategoriesSchema
        # instancedBySchema
        instancer
        instanceRegistry
        # instancerTopologySchema
        # instanceSchema
        # legacyDisplayStyleSchema
        # legacyPrimSceneIndex
        light
        # lightSchema
        material
        # materialBindingSchema
        # materialConnectionSchema
        # materialFilteringSceneIndexBase
        # materialNetworkSchema
        # materialNetwork2Interface
        # materialNodeSchema
        # materialSchema
        # mergingSceneIndex
        mesh
        # meshSchema
        meshTopology
        # meshTopologySchema
        meshUtil
        # noticeBatchingSceneIndex
        # overlayContainerDataSource
        perfLog
        pluginRenderDelegateUniqueHandle
        points
        # prefixingSceneIndex
        # primDataSourceOverlayCache
        primGather
        primTypeIndex
        # primvarSchema
        # primvarsSchema
        # purposeSchema
        renderBuffer
        # renderBufferSchema
        renderDelegate
        rendererPlugin
        rendererPluginHandle
        rendererPluginRegistry
        renderIndex
        renderPass
        renderPassState
        renderThread
        repr
        resource
        resourceRegistry
        retainedDataSource
        # retainedSceneIndex
        rprim
        rprimCollection
        rprimSharedData
        sceneDelegate
        sceneExtCompInputSource
        # sceneIndex
        # sceneIndexAdapterSceneDelegate
        # sceneIndexObserver
        # sceneIndexPlugin
        # sceneIndexPluginRegistry
        schema
        selection
        smoothNormals
        sortedIds
        sprim
        strategyBase
        # subdivisionTagsSchema
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
        # visibilitySchema
        volume
        volumeFieldBindingSchema
        # volumeFieldSchema
        vtBufferSource
        # xformSchema

    PUBLIC_HEADERS
        api.h
        dataSourceTypeDefs.h
        driver.h
        # materialNetworkInterface.h
        version.h

    PRIVATE_CLASSES
        extComputationContextInternal
        ${optionalPrivateClasses}

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)
