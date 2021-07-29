set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE rmanOslParser)
add_definitions(-DPRMANBUILDINGAPI)

# if(WIN32)
#     set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:libmmd.lib /NODEFAULTLIB:libirc.lib /NODEFAULTLIB:svml_dispmd.lib /NODEFAULTLIB:libdecimal.lib" )
# endif()

wabi_plugin(rmanOslParser
    LIBRARIES
        gf
        tf
        vt
        ar
        ndr
        sdr
        ${PXRCORE_LIBRARY}
        ${PRMAN_LIBRARY}        

    INCLUDE_DIRS
        ${RENDERMAN_INCLUDE_DIR}

    PRIVATE_CLASSES
        rmanOslParser

    RESOURCE_FILES
        resources/plugInfo.json
    
    DISABLE_PRECOMPILED_HEADERS
)

# Need to set RMANTREE and RMAN_SHADERPATH environment variable when running
# tests.
#
# XXX: RMAN_SHADERPATH is only necessary for the monolithic build; in all
# other cases, the shaders accessed by the test are automatically found
# relative to libHdPrmanLoader. We may be able to remove this if we made
# it so this library wasn't embedded in the monolithic library.
get_filename_component(RMANTREE_PATH "${RENDERMAN_INCLUDE_DIR}/.." ABSOLUTE)

if(UNIX)
    get_filename_component(RMANSHADER_PATH "/usr/local/share/kraken/1.50/datafiles/plugin/maelstrom/resources/shaders" ABSOLUTE)
elseif(WIN32)
    get_filename_component(RMANSHADER_PATH "${TARGETDIR_VER}/datafiles/plugin/maelstrom/resources/shaders" ABSOLUTE)
endif()

