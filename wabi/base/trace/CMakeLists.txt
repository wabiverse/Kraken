set(WABI_PREFIX wabi/base)
set(WABI_PACKAGE trace)

wabi_library(trace
    LIBRARIES
        arch
        js
        tf
        ${Boost_PYTHON_LIBRARY}
        ${TBB_LIBRARIES}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        aggregateTree
        aggregateNode
        category
        collection
        collectionNotice
        collector
        counterAccumulator
        dataBuffer
        dynamicKey
        event
        eventContainer
        eventData
        eventList
        eventNode
        eventTree
        key
        reporter
        reporterBase
        reporterDataSourceBase
        reporterDataSourceCollection
        reporterDataSourceCollector
        serialization
        staticKeyData
        threads

    PRIVATE_CLASSES
        aggregateTreeBuilder
        eventTreeBuilder
        jsonSerialization

    PUBLIC_HEADERS
        api.h
        concurrentList.h
        stringHash.h
        trace.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapAggregateNode.cpp
        wrapCollector.cpp
        wrapReporter.cpp
        wrapTestTrace.cpp

    PYMODULE_FILES
        python/__init__.py
        python/__main__.py

    DOXYGEN_FILES
        docs/overview.dox
        docs/detailedOverview.dox
)
