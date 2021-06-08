# - ProRender finder module
# This module searches for a valid ProRender installation, by looking at
# the ProRender_HOME environment variable. For backward compatibility reasons
# we also support PRORENDER_HOME.
#
# Variables that will be defined:
# PRORENDER_FOUND              Defined if a ProRender installation has been detected
# PRORENDER_LIBRARY            Path to ai library (for backward compatibility)
# PRORENDER_LIBRARIES          Path to ai library
# PRORENDER_INCLUDE_DIR        Path to the include directory (for backward compatibility)
# PRORENDER_INCLUDE_DIRS       Path to the include directory
#
#
# Naming convention:
#  Local variables of the form _cycles_foo
#  Input variables from CMake of the form PRORENDER_FOO
#  Output variables of the form PRORENDER_FOO
#

find_library(PRORENDER_HYBRID_LIBRARY
    NAMES
        Hybrid
        hybrid
        Hybrid.so
    HINTS
        "${ProRender_HOME}"
        "$ENV{ProRender_HOME}"
        "${PRORENDER_HOME}"
        "$ENV{PRORENDER_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "ProRender Hybrid library")

find_library(PRORENDER_NORTHSTAR_LIBRARY
    NAMES
      Northstar64
    HINTS
        "${ProRender_HOME}"
        "$ENV{ProRender_HOME}"
        "${PRORENDER_HOME}"
        "$ENV{PRORENDER_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "ProRender Northstar64 library")

find_library(PRORENDER_PRORENDER_LIBRARY
    NAMES
      RadeonProRender64
    HINTS
        "${ProRender_HOME}"
        "$ENV{ProRender_HOME}"
        "${PRORENDER_HOME}"
        "$ENV{PRORENDER_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "ProRender RadeonProRender64 library")

find_library(PRORENDER_RPRLOADSTORE_LIBRARY
    NAMES
      RprLoadStore64
    HINTS
        "${ProRender_HOME}"
        "$ENV{ProRender_HOME}"
        "${PRORENDER_HOME}"
        "$ENV{PRORENDER_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "ProRender RprLoadStore64 library")

find_library(PRORENDER_TAHOE_LIBRARY
    NAMES
        Tahoe64
    HINTS
        "${ProRender_HOME}"
        "$ENV{ProRender_HOME}"
        "${PRORENDER_HOME}"
        "$ENV{PRORENDER_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "ProRender Tahoe64 library")

find_library(PRORENDER_MIOPEN_LIBRARY
    NAMES
        MIOpen
    HINTS
        "${ProRender_HOME}"
        "$ENV{ProRender_HOME}"
        "${PRORENDER_HOME}"
        "$ENV{PRORENDER_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "ProRender MIOpen library")

find_library(PRORENDER_MLMIOPEN_LIBRARY
    NAMES
        RadeonML_MIOpen
    HINTS
        "${ProRender_HOME}"
        "$ENV{ProRender_HOME}"
        "${PRORENDER_HOME}"
        "$ENV{PRORENDER_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "ProRender RadeonML_MIOpen library")

find_library(PRORENDER_OPENIMAGEDENOISE_LIBRARY
    NAMES
        OpenImageDenoise
    HINTS
        "${ProRender_HOME}"
        "$ENV{ProRender_HOME}"
        "${PRORENDER_HOME}"
        "$ENV{PRORENDER_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "ProRender OpenImageDenoise library")

find_library(PRORENDER_IMAGEFILTERS_LIBRARY
    NAMES
        RadeonImageFilters
    HINTS
        "${ProRender_HOME}"
        "$ENV{ProRender_HOME}"
        "${PRORENDER_HOME}"
        "$ENV{PRORENDER_HOME}"
    PATH_SUFFIXES
        bin/ lib/
    DOC "ProRender RadeonImageFilters library")



list(APPEND PRORENDER_LIBRARY
    ${PRORENDER_HYBRID_LIBRARY}
    ${PRORENDER_NORTHSTAR_LIBRARY}
    ${PRORENDER_PRORENDER_LIBRARY}
    ${PRORENDER_RPRLOADSTORE_LIBRARY}
    ${PRORENDER_TAHOE_LIBRARY}
    ${PRORENDER_MIOPEN_LIBRARY}
    ${PRORENDER_MLMIOPEN_LIBRARY}
    ${PRORENDER_OPENIMAGEDENOISE_LIBRARY}
    ${PRORENDER_IMAGEFILTERS_LIBRARY}
)

find_path(PRORENDER_PRORENDER_INCLUDE_DIR RadeonProRender.h
    HINTS
        "${ProRender_HOME}"
        "$ENV{ProRender_HOME}"
        "${PRORENDER_HOME}"
        "$ENV{PRORENDER_HOME}"
    PATH_SUFFIXES
        include
    DOC "ProRender include path")

find_path(PRORENDER_RPRTOOLS_INCLUDE_DIR RadeonProRender.hpp
    HINTS
        "${ProRender_HOME}"
        "$ENV{ProRender_HOME}"
        "${PRORENDER_HOME}"
        "$ENV{PRORENDER_HOME}"
    PATH_SUFFIXES
        include/rprTools
    DOC "ProRender rprTools include path")

find_path(PRORENDER_MATH_INCLUDE_DIR half.h
  HINTS
      "${ProRender_HOME}"
      "$ENV{ProRender_HOME}"
      "${PRORENDER_HOME}"
      "$ENV{PRORENDER_HOME}"
  PATH_SUFFIXES
      include/Math
  DOC "ProRender Math include path")

set(PRORENDER_INCLUDE_DIR
    ${PRORENDER_PRORENDER_INCLUDE_DIR}
    ${PRORENDER_MATH_INCLUDE_DIR}
    ${PRORENDER_RPRTOOLS_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(ProRender
    REQUIRED_VARS
    PRORENDER_LIBRARY
    PRORENDER_INCLUDE_DIR)

set(PRORENDER_LIBRARIES ${PRORENDER_LIBRARY})
set(PRORENDER_INCLUDE_DIRS ${PRORENDER_INCLUDE_DIR})
