set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hdPhoenix)

if (NOT ${WITH_GPU_SUPPORT})
    message(STATUS
        "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT is OFF")
    return()
endif()

set(optionalPublicClasses "")

if (OPENSUBDIV_HAS_GLSL_TRANSFORM_FEEDBACK)
    add_definitions(-DOPENSUBDIV_HAS_GLSL_TRANSFORM_FEEDBACK)
endif()
if (OPENSUBDIV_HAS_GLSL_COMPUTE)
    add_definitions(-DOPENSUBDIV_HAS_GLSL_COMPUTE)
endif()

wabi_plugin(hdPhoenix
    LIBRARIES
        plug
        tf
        trace
        vt
        work
        hd
        hdPh
        ${OPENSUBDIV_LIBRARIES}
        ${TBB_LIBRARIES}

    INCLUDE_DIRS
        ${TBB_INCLUDE_DIRS}
        ${OPENSUBDIV_INCLUDE_DIRS}

    PUBLIC_CLASSES
        rendererPlugin

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)
