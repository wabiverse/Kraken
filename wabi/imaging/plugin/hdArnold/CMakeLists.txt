set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hdArnold)

if(NOT ARNOLD_FOUND)
  message(WARNING "Not building ${WABI_PACKAGE} because of missing dependency: Arnold")
  return()
endif()

wabi_plugin(hdArnold
    LIBRARIES
        arch
        plug
        tf
        hd
        ${ARNOLD_LIBRARIES}
 
    INCLUDE_DIRS
        common
        ${ARNOLD_INCLUDE_DIRS}

    CPPFILES
        nodes/driver_aov.cpp
        nodes/driver_main.cpp
        nodes/driver.cpp
        nodes/nodes.cpp

    PRIVATE_CLASSES
        basis_curves
        camera
        config
        debug_codes
        instancer
        light
        material_tracker
        material
        mesh
        native_rprim
        openvdb_asset
        points
        render_buffer
        render_delegate
        render_param
        render_pass
        renderer_plugin
        shape
        utils
        volume
        common/common_utils
        common/constant_strings
        common/shape_utils

    PRIVATE_HEADERS
        api.h
        hdarnold.h
        rprim.h
        nodes/nodes.h
        common/common_bits.h

    RESOURCE_FILES
        resources/plugInfo.json
)

add_definitions(-DUSD_HAS_UPDATED_COMPOSITOR)
add_definitions(-DUSD_HAS_FULLSCREEN_SHADER)
