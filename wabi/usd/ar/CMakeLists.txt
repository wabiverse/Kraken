set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE ar)

if(WABI_USE_AR_2)
  set(AR_VERSION 2)
else()
  set(AR_VERSION 1)
endif()

configure_file(
  ar.h
  ${CMAKE_BINARY_DIR}/include/wabi/usd/ar/ar.h)
install(
  FILES ${CMAKE_BINARY_DIR}/include/wabi/usd/ar/ar.h
  DESTINATION include/wabi/usd/ar
)

set(versionedCPPFiles "")
set(versionedPrivateHeaders "")
set(versionedPythonHeaders "")
set(versionedPythonCPPFiles "")
set(versionedPymoduleCPPFiles "")

list(APPEND versionedPrivateHeaders
    asset_v2.h
    defaultResolver_v2.h
    filesystemAsset_v2.h
    resolver_v2.h
    resolverContext_v2.h
)

list(APPEND versionedCPPFiles
      asset_v2.cpp
      defaultResolver_v2.cpp
      filesystemAsset_v2.cpp
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
        filesystemWritableAsset.h
        definePackageResolver.h
        defineResolver.h
        defineResolverContext.h
        notice.h
        packageResolver.h
        packageUtils.h
        resolvedPath.h
        resolver.h
        resolverContext.h
        resolverContextBinder.h
        resolverScopedCache.h
        threadLocalScopedCache.h
        timestamp.h
        writableAsset.h

    PYTHON_PUBLIC_HEADERS
        pyResolverContext.h

    PYTHON_PRIVATE_HEADERS
        ${versionedPythonHeaders}

    PRIVATE_HEADERS
        debugCodes.h
        ${versionedPrivateHeaders}

    CPPFILES
        assetInfo.cpp
        notice.cpp
        packageResolver.cpp
        packageUtils.cpp
        resolvedPath.cpp
        resolverContextBinder.cpp
        resolverScopedCache.cpp
        timestamp.cpp
        debugCodes.cpp
        defaultResolverContext.cpp
        ${versionedCPPFiles}

    PYTHON_CPPFILES
        moduleDeps.cpp
        ${versionedPythonCPPFiles}

    PYMODULE_CPPFILES
        module.cpp
        wrapDefaultResolverContext.cpp
        wrapNotice.cpp
        wrapPackageUtils.cpp
        wrapResolvedPath.cpp
        wrapResolverContextBinder.cpp
        wrapResolverScopedCache.cpp
        wrapTimestamp.cpp
        ${versionedPymoduleCPPFiles}

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)
