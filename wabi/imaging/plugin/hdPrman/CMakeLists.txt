# Install hdPrman's headers in wabi/imaging alongside other shared
# libraries for Hydra backends, like hdSt.
set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hdPrman)

# RiEntryPoints.h incorrectly makes libloadprman symbols DLL import
# This is only temporary and will be removed shortly
add_definitions(-DPRMANBUILDINGAPI)

# TODO The wabi cmake macros don't provide a good way to link prman with link_directories
# on Linux/OSX without resorting to absolute rpath. For now, simply avoid linking
# libprman on Linux/OSX.
set(optionalLibs "")
if(WIN32)
    # set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:libmmd.lib /NODEFAULTLIB:libirc.lib /NODEFAULTLIB:svml_dispmd.lib /NODEFAULTLIB:libdecimal.lib" )
    list(APPEND optionalLibs ${PXRCORE_LIBRARY})
    list(APPEND optionalLibs ${PRMAN_LIBRARY})
elseif(APPLE)
    set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup -headerpad_max_install_names" )
endif()

set(optionalIncludeDirs "")
set(optionalPublicClasses "")
if (WITH_MATERIALX)
    add_definitions(-DWABI_MATERIALX_STDLIB_DIR="${MATERIALX_STDLIB_DIR}")
    add_definitions(-DWABI_MATERIALX_BASE_DIR="${MATERIALX_BASE_DIR}")

    list(APPEND optionalLibs ${MATERIALX_LIBRARIES} hdMtlx)
    list(APPEND optionalIncludeDirs ${MATERIALX_INCLUDE_DIRS})
    list(APPEND optionalPublicClasses matfiltMaterialX)

    if (WITH_OSL)
        list(APPEND optionalLibs ${OSL_COMP_LIBRARY} ${OPENEXR_LIBRARIES} ${OIIO_LIBRARIES})
        list(APPEND optionalIncludeDirs ${OSL_INCLUDE_DIR} ${OPENEXR_INCLUDE_DIR} ${OIIO_INCLUDE_DIRS})
    endif()
endif()

wabi_plugin(${WABI_PACKAGE}
    LIBRARIES
        ar
        plug
        tf
        vt
        gf
        hd
        usdRi
        usdVol
        ${optionalLibs}

    INCLUDE_DIRS
        ${RENDERMAN_INCLUDE_DIR}
        ${optionalIncludeDirs}

    PUBLIC_CLASSES
        basisCurves
        camera
        context
        coordSys
        debugCodes
        gprim
        instancer
        light
        lightFilter
        lightFilterUtils
        material
        matfiltConvertPreviewMaterial
        matfiltFilterChain
        matfiltResolveVstructs
        mesh
        points
        renderDelegate
        renderPass
        rixStrings
        volume
        xcpt
        ${optionalPublicClasses}

    PUBLIC_HEADERS
        api.h
        renderParam.h

    PRIVATE_HEADERS
        virtualStructConditionalGrammar.tab.h

    CPPFILES
        virtualStructConditionalGrammar.tab.cpp
        virtualStructConditionalGrammar.lex.cpp

    DISABLE_PRECOMPILED_HEADERS
)
