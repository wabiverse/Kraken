set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hdCycles)

if(NOT CYCLES_FOUND)
  message(WARNING "Not building ${WABI_PACKAGE} because of missing dependency: Cycles")
  return()
endif()

add_definitions(
  -DCCL_NAMESPACE_BEGIN=namespace\ ccl\ {
  -DCCL_NAMESPACE_END=}
)

wabi_plugin(hdCycles
    LIBRARIES
        plug
        tf
        vt
        gf
        work
        sdf
        hf
        hd
        hdx
        usdLux
        pxOsd
        cameraUtil
        ${EMBREE_LIBRARY}
        ${TBB_LIBRARIES}
        ${OCIO_LIBRARIES}
        ${OIIO_LIBRARIES}
        ${OSL_EXEC_LIBRARY}
        ${OSL_COMP_LIBRARY}
        ${OSL_QUERY_LIBRARY}
        ${OPENIMAGEDENOISE_LIBRARIES}
        ${CYCLES_LIBRARIES}
        ${ALL_IMAGING_LIBS}
        ${OPENVDB_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
 
    INCLUDE_DIRS
        Mikktspace
        ${OPENEXR_INCLUDE_DIR}
        ${EMBREE_INCLUDE_DIR}
        ${TBB_INCLUDE_DIRS}
        ${OCIO_INCLUDE_DIRS}
        ${OIIO_INCLUDE_DIRS}
        ${OSL_INCLUDE_DIR}
        ${OPENIMAGEDENOISE_INCLUDE_DIRS}
        ${OPENVDB_INCLUDE_DIR}
        ${CYCLES_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${LIBDIR}/cycles/include/third_party/atomic

    CPPFILES
        Mikktspace/mikktspace.c

    PRIVATE_CLASSES
        attributeSource
        basisCurves
        camera
        config
        debug_codes
        instancer
        light
        material
        mesh
        meshRefiner
        meshSource
        objectSource
        openvdb_asset
        points
        renderBuffer
        renderDelegate
        rendererPlugin
        renderParam
        renderPass
        renderPassState
        resourceRegistry
        tokens
        transformSource
        utils
        volume

    PRIVATE_HEADERS
        api.h
        hdcycles.h

    RESOURCE_FILES
        resources/plugInfo.json

    DISABLE_PRECOMPILED_HEADERS
)
