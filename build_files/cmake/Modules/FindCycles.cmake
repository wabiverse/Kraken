# - Cycles finder module
# This module searches for a valid Cycles installation, by looking at
# the Cycles_HOME environment variable. For backward compatibility reasons
# we also support CYCLES_HOME.
#
# Variables that will be defined:
# CYCLES_FOUND              Defined if a Cycles installation has been detected
# CYCLES_LIBRARY            Path to ai library (for backward compatibility)
# CYCLES_LIBRARIES          Path to ai library
# CYCLES_INCLUDE_DIR        Path to the include directory (for backward compatibility)
# CYCLES_INCLUDE_DIRS       Path to the include directory
#
#
# Naming convention:
#  Local variables of the form _cycles_foo
#  Input variables from CMake of the form CYCLES_FOO
#  Output variables of the form CYCLES_FOO
#

find_library(CYCLES_BVH_LIBRARY
    NAMES
        cycles_bvh
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles bvh library")

find_library(CYCLES_DEVICE_LIBRARY
    NAMES
        cycles_device
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles device library")

find_library(CYCLES_GRAPH_LIBRARY
    NAMES
        cycles_graph
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles graph library")

find_library(CYCLES_KERNEL_LIBRARY
    NAMES
        cycles_kernel
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles kernel library")

find_library(CYCLES_KERNEL_OSL_LIBRARY
    NAMES
        cycles_kernel_osl
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles kernel osl library")

find_library(CYCLES_RENDER_LIBRARY
    NAMES
        cycles_render
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles render library")

find_library(CYCLES_SUBD_LIBRARY
    NAMES
        cycles_subd
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles subd library")

find_library(CYCLES_UTIL_LIBRARY
    NAMES
        cycles_util
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles util library")

find_library(CYCLES_CLEW_LIBRARY
    NAMES
        extern_clew
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles clew library")

find_library(CYCLES_CUEW_LIBRARY
    NAMES
        extern_cuew
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles cuew library")

find_library(CYCLES_LIBC_COMPAT_LIBRARY
    NAMES
        extern_libc_compat
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles libc compat library")

find_library(CYCLES_NUMAAPI_LIBRARY
    NAMES
        extern_numaapi
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles numaapi library")

find_library(CYCLES_SKY_LIBRARY
    NAMES
        extern_sky
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "Cycles sky library")

list(APPEND CYCLES_LIBRARY
    ${CYCLES_BVH_LIBRARY}
    ${CYCLES_DEVICE_LIBRARY}
    ${CYCLES_GRAPH_LIBRARY}
    ${CYCLES_KERNEL_LIBRARY}
    ${CYCLES_KERNEL_OSL_LIBRARY}
    ${CYCLES_RENDER_LIBRARY}
    ${CYCLES_SUBD_LIBRARY}
    ${CYCLES_UTIL_LIBRARY}
    ${CYCLES_CLEW_LIBRARY}
    ${CYCLES_CUEW_LIBRARY}
    ${CYCLES_LIBC_COMPAT_LIBRARY}
    ${CYCLES_NUMAAPI_LIBRARY}
    ${CYCLES_SKY_LIBRARY}
)

find_path(CYCLES_APP_INCLUDE_DIR cycles_xml.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/app
    DOC "Cycles app include path")

find_path(CYCLES_BLENDER_INCLUDE_DIR blender_image.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/blender
    DOC "Cycles blender include path")

find_path(CYCLES_BVH_INCLUDE_DIR bvh.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/bvh
    DOC "Cycles bvh include path")

find_path(CYCLES_DEVICE_INCLUDE_DIR device.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/device
    DOC "Cycles device include path")

find_path(CYCLES_GRAPH_INCLUDE_DIR node.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/graph
    DOC "Cycles graph include path")

find_path(CYCLES_KERNEL_INCLUDE_DIR kernel_types.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/kernel
    DOC "Cycles kernel include path")

find_path(CYCLES_RENDER_INCLUDE_DIR attribute.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/render
    DOC "Cycles render include path")

find_path(CYCLES_SUBD_INCLUDE_DIR subd_dice.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/subd
    DOC "Cycles subd include path")

find_path(CYCLES_UTIL_INCLUDE_DIR util_half.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/util
    DOC "Cycles util include path")

find_path(CYCLES_ATOMIC_INCLUDE_DIR atomic_ops.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/atomic
    DOC "Cycles atomic include path")

find_path(CYCLES_CLEW_INCLUDE_DIR clew.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/clew/include
    DOC "Cycles clew include path")

find_path(CYCLES_CUEW_INCLUDE_DIR cuew.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/cuew/include
    DOC "Cycles cuew include path")

find_path(CYCLES_NUMAAPI_INCLUDE_DIR numaapi.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/numaapi/include
    DOC "Cycles numaapi include path")

find_path(CYCLES_SKY_INCLUDE_DIR sky_model.h
    HINTS
        "${Cycles_HOME}"
        "$ENV{Cycles_HOME}"
        "${CYCLES_HOME}"
        "$ENV{CYCLES_HOME}"
    PATH_SUFFIXES
        include/sky/include
    DOC "Cycles sky include path")

set(CYCLES_INCLUDE_DIR
    ${CYCLES_APP_INCLUDE_DIR}
    ${CYCLES_BLENDER_INCLUDE_DIR}
    ${CYCLES_BVH_INCLUDE_DIR}
    ${CYCLES_DEVICE_INCLUDE_DIR}
    ${CYCLES_GRAPH_INCLUDE_DIR}
    ${CYCLES_KERNEL_INCLUDE_DIR}
    ${CYCLES_RENDER_INCLUDE_DIR}
    ${CYCLES_SUBD_INCLUDE_DIR}
    ${CYCLES_UTIL_INCLUDE_DIR}
    # Third Parties
    ${CYCLES_ATOMIC_INCLUDE_DIR}
    ${CYCLES_CLEW_INCLUDE_DIR}
    ${CYCLES_CUEW_INCLUDE_DIR}
    ${CYCLES_NUMAAPI_INCLUDE_DIR}
    ${CYCLES_SKY_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

if(WIN32)
    set(CYCLES_INCLUDE_DIR ${CYCLES_HOME}/include)
    list(APPEND CYCLES_LIBRARY
      ${CYCLES_HOME}/lib/cycles_bvh.lib
      ${CYCLES_HOME}/lib/cycles_device.lib
      ${CYCLES_HOME}/lib/cycles_graph.lib
      ${CYCLES_HOME}/lib/cycles_kernel_osl.lib
      ${CYCLES_HOME}/lib/cycles_kernel.lib
      ${CYCLES_HOME}/lib/cycles_render.lib
      ${CYCLES_HOME}/lib/cycles_subd.lib
      ${CYCLES_HOME}/lib/cycles_util.lib
      ${CYCLES_HOME}/lib/extern_clew.lib
      ${CYCLES_HOME}/lib/extern_cuew.lib
      ${CYCLES_HOME}/lib/extern_numaapi.lib
      ${CYCLES_HOME}/lib/extern_sky.lib
    )
else()
    find_package_handle_standard_args(Cycles
        REQUIRED_VARS
        CYCLES_LIBRARY
        CYCLES_INCLUDE_DIR)
endif()

set(CYCLES_LIBRARIES ${CYCLES_LIBRARY})
set(CYCLES_INCLUDE_DIRS ${CYCLES_INCLUDE_DIR})
