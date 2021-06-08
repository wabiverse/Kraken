set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hdEmbree)

if(NOT EMBREE_FOUND)
    message(WARNING "Not building ${WABI_PACKAGE} because of missing dependency: Embree")
    return()
endif()
if (NOT ${WITH_GPU_SUPPORT})
    message(STATUS
        "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT is OFF")
    return()
endif()

wabi_plugin(hdEmbree
   LIBRARIES
        plug
        tf
        vt
        gf
        work
        hf
        hd
        hdx
        ${TBB_LIBRARIES}
        ${EMBREE_LIBRARY}

    INCLUDE_DIRS
        ${TBB_INCLUDE_DIRS}
        ${EMBREE_INCLUDE_DIR}

    PUBLIC_CLASSES
        config
        instancer
        mesh
        meshSamplers
        renderBuffer
        renderer
        rendererPlugin
        renderDelegate
        renderPass
        sampler

    PUBLIC_HEADERS
        context.h
        renderParam.h

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)
