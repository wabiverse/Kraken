# Make sure not to get the wrong semaphore.h
set(CMAKE_INCLUDE_CURRENT_DIR OFF)

set(WABI_PACKAGE tf)
set(WABI_PREFIX wabi/base)

if(WIN32)
    set(WINLIBS Shlwapi.lib)
endif()

wabi_library(tf
    LIBRARIES
        arch
        ${WINLIBS}
        ${PYTHON_LIBRARIES}
        ${Boost_PYTHON_LIBRARY}
        ${TBB_tbb_LIBRARY}

    INCLUDE_DIRS
        ${PYTHON_INCLUDE_DIRS}
        ${PYTHON_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        anyUniquePtr
        anyWeakPtr
        atomicOfstreamWrapper
        bitUtils
        debug
        debugNotice
        denseHashMap
        denseHashSet
        diagnostic
        diagnosticBase
        diagnosticHelper
        diagnosticMgr
        dl
        enum
        envSetting
        error
        errorMark
        errorTransport
        exception
        expiryNotifier
        fastCompression
        fileUtils
        getenv
        hash
        iterator
        mallocTag
        notice
        nullPtr
        ostreamMethods
        pathUtils
        patternMatcher
        pointerAndBits
        pyLock
        pyObjWrapper
        pyTracing
        refBase
        refCount
        refPtr
        refPtrTracker
        regTest
        registryManager
        safeOutputFile
        scoped
        scopeDescription
        setenv
        singleton
        smallVector
        stackTrace
        stacked
        status
        stl
        stopwatch
        stringUtils
        templateString
        tf
        token
        type
        typeFunctions
        typeNotice
        warning
        weakBase
        weakPtr
        weakPtrFacade

    PYTHON_PUBLIC_CLASSES
        makePyConstructor
        pyAnnotatedBoolResult
        pyArg
        pyCall
        pyCallContext
        pyClassMethod
        pyContainerConversions
        pyEnum
        pyError
        pyExceptionState
        pyFunction
        pyIdentity
        pyInterpreter
        pyInvoke
        pyModule
        pyModuleNotice
        pyNoticeWrapper
        pyObjectFinder
        pyOptional
        pyOverride
        pyPolymorphic
        pyPtrHelpers
        pyResultConversions
        pySingleton
        pyUtils
        pyWrapContext
        scriptModuleLoader
        wrapTypeHelpers

    PUBLIC_HEADERS
        api.h
        callContext.h
        cxxCast.h
        declarePtrs.h
        diagnosticLite.h
        functionRef.h
        hashmap.h
        hashset.h
        instantiateSingleton.h
        instantiateStacked.h
        instantiateType.h
        pxrTslRobinMap/robin_growth_policy.h
        pxrTslRobinMap/robin_hash.h
        pxrTslRobinMap/robin_map.h
        pxrTslRobinMap/robin_set.h
        preprocessorUtils.h
        preprocessorUtilsLite.h
        safeTypeCompare.h
        span.h
        staticData.h
        staticTokens.h
        typeInfoMap.h
        type_Impl.h

    PYTHON_PUBLIC_HEADERS
        py3Compat.h
        pySafePython.h
        pyStaticTokens.h

    PRIVATE_CLASSES
        atomicRenameUtil
        debugCodes
        noticeRegistry

    PYTHON_PRIVATE_CLASSES
        pyErrorInternal

    PRIVATE_HEADERS
        scopeDescriptionPrivate.h
        pixarDoubleConversion/ieee.h
        pixarDoubleConversion/utils.h
        pixarDoubleConversion/double-conversion.h
        pixarDoubleConversion/bignum-dtoa.h
        pixarDoubleConversion/bignum.h
        pixarDoubleConversion/cached-powers.h
        pixarDoubleConversion/diy-fp.h
        pixarDoubleConversion/fast-dtoa.h
        pixarDoubleConversion/fixed-dtoa.h
        pixarDoubleConversion/strtod.h
        pixarLZ4/lz4.h
        pyWeakObject.h

    CPPFILES
        initConfig.cpp
        preprocessorUtils.cpp
        pixarDoubleConversion/double-conversion.cc
        pixarDoubleConversion/bignum-dtoa.cc
        pixarDoubleConversion/bignum.cc
        pixarDoubleConversion/cached-powers.cc
        pixarDoubleConversion/diy-fp.cc
        pixarDoubleConversion/fast-dtoa.cc
        pixarDoubleConversion/fixed-dtoa.cc
        pixarDoubleConversion/strtod.cc
        pixarLZ4/lz4.cpp

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        pyWeakObject.cpp
        wrapAnyWeakPtr.cpp
        wrapCallContext.cpp
        wrapDebug.cpp
        wrapDiagnostic.cpp
        wrapDiagnosticBase.cpp
        wrapEnum.cpp
        wrapEnvSetting.cpp
        wrapError.cpp
        wrapException.cpp
        wrapFileUtils.cpp
        wrapFunction.cpp
        wrapMallocTag.cpp
        wrapNotice.cpp
        wrapPathUtils.cpp
        wrapPyContainerConversions.cpp
        wrapPyModuleNotice.cpp
        wrapPyObjWrapper.cpp
        wrapPyOptional.cpp
        wrapRefPtrTracker.cpp
        wrapScopeDescription.cpp
        wrapScriptModuleLoader.cpp
        wrapSingleton.cpp
        wrapStackTrace.cpp
        wrapStatus.cpp
        wrapStopwatch.cpp
        wrapStringUtils.cpp
        wrapTemplateString.cpp
        wrapTestPyAnnotatedBoolResult.cpp
        wrapTestPyContainerConversions.cpp
        wrapTestPyStaticTokens.cpp
        wrapTestTfPython.cpp
        wrapTestTfPyOptional.cpp
        wrapToken.cpp
        wrapType.cpp
        wrapWarning.cpp


    PYMODULE_FILES 
        python/__init__.py
        # testenv/__init__.py
        # testenv/testTfScriptModuleLoader_AAA_RaisesError.py
        # testenv/testTfScriptModuleLoader_DepLoadsAll.py
        # testenv/testTfScriptModuleLoader_LoadsAll.py
        # testenv/testTfScriptModuleLoader_LoadsUnknown.py
        # testenv/testTfScriptModuleLoader_Other.py
        # testenv/testTfScriptModuleLoader_Test.py
        # testenv/testTfScriptModuleLoader_Unknown.py
        # testenv/testTfPyInvoke_callees.py

    DOXYGEN_FILES
        docs/diagnosticOverview.dox
        docs/errorOverview.dox
        docs/mallocTagOverview.dox
        docs/noteOverview.dox
        docs/overview.dox
        docs/registryOverview.dox
)