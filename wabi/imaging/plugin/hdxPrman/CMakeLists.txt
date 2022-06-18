set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hdxPrman)

# TODO The wabi cmake macros don't provide a good way to link prman with link_directories
# on Linux/OSX without resorting to absolute rpath. For now, simply avoid linking
# libprman on Linux/OSX.
set(optionalLibs "")
if(WIN32)
    list(APPEND optionalLibs ${PXRCORE_LIBRARY})
    list(APPEND optionalLibs ${PRMAN_LIBRARY})
elseif(APPLE)
    set( CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -undefined dynamic_lookup -headerpad_max_install_names" )
endif()
wabi_plugin(${WABI_PACKAGE}
    LIBRARIES
        hd
        work
        ${optionalLibs}
        hdPrman

    INCLUDE_DIRS
        ${RENDERMAN_INCLUDE_DIR}

    PRIVATE_CLASSES
        context
        framebuffer
        renderBuffer
        renderDelegate
        renderPass
        resourceRegistry

    CPPFILES
        rendererPlugin.cpp

    PRIVATE_HEADERS
        renderParam.h

    RESOURCE_FILES
        resources/plugInfo.json
)
