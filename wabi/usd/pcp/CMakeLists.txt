set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE pcp)

wabi_library(pcp
    LIBRARIES
        tf
        trace
        vt
        sdf
        work
        ar
        ${Boost_PYTHON_LIBRARY}
        ${TBB_tbb_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}
        
    PUBLIC_HEADERS
        api.h

    PUBLIC_CLASSES
        arc
        cache
        changes
        composeSite
        dependency
        dynamicFileFormatContext
        dynamicFileFormatDependencyData
        dynamicFileFormatInterface
        errors
        instanceKey
        iterator
        layerPrefetchRequest
        layerStack
        layerStackIdentifier
        mapExpression
        mapFunction
        namespaceEdits
        node
        pathTranslation
        primIndex
        propertyIndex
        site
        strengthOrdering
        targetIndex
        types

    PYTHON_PUBLIC_CLASSES
        pyUtils

    PRIVATE_CLASSES
        debugCodes
        dependencies
        diagnostic
        instancing
        layerStackRegistry
        node_Iterator
        primIndex_Graph
        primIndex_StackFrame
        statistics
        utils

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapCache.cpp
        wrapDependency.cpp
        wrapDynamicFileFormatDependencyData.cpp
        wrapErrors.cpp
        wrapInstanceKey.cpp
        wrapPrimIndex.cpp
        wrapLayerStack.cpp
        wrapLayerStackIdentifier.cpp
        wrapMapExpression.cpp
        wrapMapFunction.cpp
        wrapNode.cpp
        wrapPathTranslation.cpp
        wrapPropertyIndex.cpp
        wrapSite.cpp
        wrapTestChangeProcessor.cpp
        wrapTypes.cpp

    PYMODULE_FILES
        python/__init__.py

    DOXYGEN_FILES
        docs/overview.dox
)
