set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE ar)

configure_file(
    resources/ar.h.in
    ${CMAKE_BINARY_DIR}/include/wabi/usd/ar/ar.h)

if(UNIX)
    install(
        FILES ${CMAKE_BINARY_DIR}/include/wabi/usd/ar/ar.h
        DESTINATION "/usr/local/share/kraken/${TARGETDIR_VER}/include/wabi/usd/ar"
    )
elseif(WIN32)
    install(
        FILES ${CMAKE_BINARY_DIR}/include/wabi/usd/ar/ar.h
        DESTINATION "${TARGETDIR_VER}/include/wabi/usd/ar"
    )   
endif()

set(versionedHeaders "")
set(versionedCPPFiles "")
set(versionedPrivateHeaders "")
set(versionedPythonHeaders "")
set(versionedPythonCPPFiles "")
set(versionedPymoduleCPPFiles "")


if (WITH_PIXAR_AR_BETA)
    list(APPEND versionedHeaders 
         defaultResolver_v2.h
         filesystemWritableAsset.h
         resolver_v2.h
         resolverContext_v2.h
         writableAsset.h
    )
    list(APPEND versionedCPPFiles
         defaultResolver_v2.cpp
         filesystemWritableAsset.cpp
         resolver_v2.cpp
         resolverContext_v2.cpp
         writableAsset.cpp
    )
    list(APPEND versionedPythonHeaders
         pyResolverContext_v2.h
    )
    list(APPEND versionedPythonCPPFiles
         pyResolverContext_v2.cpp
    )
    list(APPEND versionedPymoduleCPPFiles
         wrapDefaultResolver_v2.cpp
         wrapResolver_v2.cpp
         wrapResolverContext_v2.cpp
    )
else()
    list(APPEND versionedHeaders 
         defaultResolver_v1.h
         resolver_v1.h
         resolverContext_v1.h
    )
    list(APPEND versionedCPPFiles
         defaultResolver_v1.cpp
         resolver_v1.cpp
         resolverContext_v1.cpp
    )
    list(APPEND versionedPythonHeaders
         pyResolverContext_v1.h
    )
    list(APPEND versionedPythonCPPFiles
         pyResolverContext_v1.cpp
    )
    list(APPEND versionedPymoduleCPPFiles
         wrapDefaultResolver_v1.cpp
         wrapResolver_v1.cpp
         wrapResolverContext_v1.cpp
    )
endif()

wabi_library(ar
    LIBRARIES
        arch
        tf
        plug
        vt
        ${Boost_PYTHON_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}

    PUBLIC_HEADERS
        api.h
        asset.h
        assetInfo.h
        defaultResolver.h
        defaultResolverContext.h
        filesystemAsset.h
        definePackageResolver.h
        defineResolver.h
        defineResolverContext.h
        packageResolver.h
        packageUtils.h
        resolvedPath.h
        resolver.h
        resolverContext.h
        resolverContextBinder.h
        resolverScopedCache.h
        threadLocalScopedCache.h
        ${versionedHeaders}

    PYTHON_PUBLIC_HEADERS
        pyResolverContext.h
        ${versionedPythonHeaders}

    PRIVATE_HEADERS
        debugCodes.h
        ${versionedPrivateHeaders}

    CPPFILES
        asset.cpp
        assetInfo.cpp
        filesystemAsset.cpp
        packageResolver.cpp
        packageUtils.cpp
        resolvedPath.cpp
        resolverContextBinder.cpp
        resolverScopedCache.cpp
        debugCodes.cpp
        defaultResolverContext.cpp
        ${versionedCPPFiles}

    PYTHON_CPPFILES
        moduleDeps.cpp
        ${versionedPythonCPPFiles}

    PYMODULE_CPPFILES
        module.cpp
        wrapDefaultResolverContext.cpp
        wrapPackageUtils.cpp
        wrapResolvedPath.cpp
        wrapResolverContextBinder.cpp
        wrapResolverScopedCache.cpp
        ${versionedPymoduleCPPFiles}

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)
