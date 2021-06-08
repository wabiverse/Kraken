set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdUtils)

wabi_library(usdUtils
    LIBRARIES
        arch
        tf
        gf
        sdf
        usd
        usdGeom
        ${Boost_PYTHON_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}

    PUBLIC_CLASSES
        authoring
        coalescingDiagnosticDelegate
        conditionalAbortDiagnosticDelegate
        debugCodes
        dependencies
        flattenLayerStack
        introspection
        pipeline
        registeredVariantSet
        sparseValueWriter
        stageCache
        stitch
        stitchClips
        timeCodeRange

    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapAuthoring.cpp
        wrapCoalescingDiagnosticDelegate.cpp
        wrapConditionalAbortDiagnosticDelegate.cpp
        wrapDependencies.cpp
        wrapFlattenLayerStack.cpp
        wrapIntrospection.cpp
        wrapPipeline.cpp
        wrapRegisteredVariantSet.cpp
        wrapSparseValueWriter.cpp
        wrapStageCache.cpp
        wrapStitch.cpp
        wrapStitchClips.cpp
        wrapTimeCodeRange.cpp

    PYMODULE_FILES
        python/__init__.py
        python/complianceChecker.py
        python/constantsGroup.py
        python/updateSchemaWithSdrNode.py

    DOXYGEN_FILES
        docs/overview.dox
)
