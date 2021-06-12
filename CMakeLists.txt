#-----------------------------------------------------------------------------
# Prevent In-Source Builds

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  if(NOT DEFINED WITH_IN_SOURCE_BUILD)
    message(FATAL_ERROR
      "CMake generation for covah is not allowed within the source directory!"
      "\n Remove \"${CMAKE_SOURCE_DIR}/CMakeCache.txt\" and try again from another folder, e.g.:"
      "\n "
      "\n rm CMakeCache.txt"
      "\n cd .."
      "\n mkdir cmake-make"
      "\n cd cmake-make"
      "\n cmake ../covah"
      "\n "
      "\n Alternately define WITH_IN_SOURCE_BUILD to force this option (not recommended!)"
    )
  endif()
endif()

#-----------------------------------------------------------------------------
# Required Initial Setup

cmake_minimum_required(VERSION 3.19)

if(NOT EXECUTABLE_OUTPUT_PATH)
  set(FIRST_RUN TRUE)
else()
  set(FIRST_RUN FALSE)
endif()

# this starts out unset
list (APPEND CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}"
                               "${CMAKE_SOURCE_DIR}/build_files/cmake"
                               "${CMAKE_SOURCE_DIR}/build_files/cmake/macros"
                               "${CMAKE_SOURCE_DIR}/build_files/cmake/platform"
                               "${CMAKE_SOURCE_DIR}/build_files/cmake/Modules")

# avoid having empty buildtype
if(NOT DEFINED CMAKE_BUILD_TYPE_INIT)
  set(CMAKE_BUILD_TYPE_INIT "Release")
endif()

# Omit superfluous "Up-to-date" messages.
if(NOT DEFINED CMAKE_INSTALL_MESSAGE)
  set(CMAKE_INSTALL_MESSAGE "LAZY")
endif()

# global compile definitions since add_definitions() adds for all.
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
  $<$<CONFIG:Debug>:DEBUG;_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
  $<$<CONFIG:MinSizeRel>:NDEBUG>
  $<$<CONFIG:RelWithDebInfo>:NDEBUG>
)

#-----------------------------------------------------------------------------
# Set policy

# see "cmake --help-policy CMP0003"
# So library linking is more sane
cmake_policy(SET CMP0003 NEW)

# So BUILDINFO and COVAHPATH strings are automatically quoted
cmake_policy(SET CMP0005 NEW)

# So syntax problems are errors
cmake_policy(SET CMP0010 NEW)

# Input directories must have CMakeLists.txt
cmake_policy(SET CMP0014 NEW)

# Silence draco warning on macOS, new policy works fine.
if(POLICY CMP0068)
  cmake_policy(SET CMP0068 NEW)
endif()

# find_package() uses <PackageName>_ROOT variables.
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

#-----------------------------------------------------------------------------
# Load some macros.

include(macros)

#-----------------------------------------------------------------------------
# Initialize project.

covah_project_hack_pre()

project(COVAH)

covah_project_hack_post()

enable_testing()

#-----------------------------------------------------------------------------
# Redirect output files

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "" FORCE)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "" FORCE)

get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(GENERATOR_IS_MULTI_CONFIG)
  set(TESTS_OUTPUT_DIR ${EXECUTABLE_OUTPUT_PATH}/tests/$<CONFIG>/ CACHE INTERNAL "" FORCE)
else()
  set(TESTS_OUTPUT_DIR ${EXECUTABLE_OUTPUT_PATH}/tests/ CACHE INTERNAL "" FORCE)
endif()

#-----------------------------------------------------------------------------
# Force colored output on Unix

if(UNIX)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
  endif()
endif()

#-----------------------------------------------------------------------------
# Set default config options

# Hack, because CMAKE provided variables are not handed down from MSVC
# and we need to ensure we are able to check Release v. Debug modes
# to link and install with the proper Release v. Debug libraries.
if(CMAKE_BINARY_DIR MATCHES ".*Release")
  option(COVAH_RELEASE_MODE "Covah build type detection" ON)
elseif(CMAKE_BINARY_DIR MATCHES ".*Debug")
  option(COVAH_RELEASE_MODE "Covah build type detection" OFF)
endif()

if(CMAKE_BUILD_TYPE_INIT STREQUAL "Debug")
  add_definitions(-DTBB_USE_DEBUG=1)
  add_definitions(-DTBB_USE_ASSERT=1)
  add_definitions(-DTBB_USE_PROFILING_TOOLS=1)
  add_definitions(-DOSL_DEBUG=1)
endif()

get_covah_version()

set(TARGETDIR_VER ${COVAH_VERSION})

#-----------------------------------------------------------------------------
# Build as shared for plugin support and user extensibility.

option(BUILD_SHARED_LIBS "Build all libraries as shared libraries" ON)

#-----------------------------------------------------------------------------
# Build Options

option(WITH_COVAH                    "Build the COVAH executable, The Digital Content Creation Platform of the 21st Century"      ON)
option(WITH_COVAH_MONOLITHIC         "Build all Pixar API libraries as one big monolithic library, the covahverse"                ON)
option(WITH_LICENSING                "Save my investments god damnit"                                                             ON)
option(WITH_TESTS                    "Build covah test executables"                                                               ON)
option(WITH_USD                      "Enable Universal Scene Description (USD) Support"                                           ON)
option(WITH_PIXAR_AR_BETA            "Enable developmental preview of Pixar Asset Resolution v2.0"                                ON)
option(WITH_PIXAR_USDVIEW            "Build the Pixar UsdView Python Application"                                                 ON)
option(WITH_PIXAR_TOOLKIT            "Build the Pixar USD tools"                                                                  ON)
option(WITH_TBB                      "Enable Threading Building Blocks Support"                                                   ON)
option(WITH_VULKAN                   "Enable Vulkan graphics support"                                                             ON)
option(WITH_OPENSUBDIV               "Enable OpenSubdiv for surface subdivision"                                                  ON)
option(WITH_OPENCOLORIO              "Enable OpenColorIO color management"                                                        ON)
option(WITH_ARNOLD                   "Enable Arnold Renderer support"                                                             ON)
option(WITH_PRORENDER                "Enable AMD ProRender support"                                                              OFF)
option(WITH_CYCLES                   "Enable Cycles Renderer support"                                                             ON)
option(WITH_RENDERMAN                "Enable RenderMan, Pixar's Render Engine"                                                    ON)
option(WITH_PHOENIX                  "Enable Phoenix, real-time render engine support"                                            ON)
option(WITH_EMBREE                   "Enable RenderMan, Pixar's Render Engine"                                                    ON)
option(WITH_ALEMBIC                  "Enable Alembic Support"                                                                     ON)
option(WITH_ALEMBIC_HDF5             "Enable HDF5 in Alembic for high performance data management and storage"                    ON)
option(WITH_DRACO                    "Enable Draco Support"                                                                       ON)
option(WITH_MATERIALX                "Enable MaterialX support for COVAH through the usdMatx plugin"                              ON)
option(WITH_OSL                      "Build with full OSL support for MaterialX, and others."                                     ON)
option(WITH_OPENIMAGEIO              "Enable OpenImageIO Support (http://www.openimageio.org)"                                    ON)
option(WITH_PTEX                     "Enable Walt Disney Animation Studios Ptex support (https://ptex.us/)"                       ON)
option(WITH_IMAGE_OPENEXR            "Enable OpenEXR Support (http://www.openexr.com)"                                            ON)
option(WITH_IMAGE_OPENJPEG           "Enable OpenJpeg Support (http://www.openjpeg.org)"                                          ON)
option(WITH_IMAGE_TIFF               "Enable TIFF Support"                                                                        ON)
option(WITH_OPENVDB                  "Enable features relying on OpenVDB"                                                         ON)
option(WITH_OPENVDB_BLOSC            "Enable blosc compression for OpenVDB, only enable if OpenVDB was built with blosc support"  ON)
option(WITH_NANOVDB                  "Enable usage of NanoVDB data structure for accelerated rendering on the GPU"               OFF)
option(WITH_PYTHON                   "Enable Embedded Python API  (only disable for development)"                                 ON)
option(WITH_PYTHON_INSTALL           "Install python and it's libraries to distribute with COVAH"                                 ON)
option(WITH_PYTHON_INSTALL_NUMPY     "Install numpy alongside python installation"                                                ON)
option(WITH_PYTHON_INSTALL_REQUESTS  "Install requests alongside python installation"                                             ON)
option(WITH_PYTHON_SECURITY          "Disables execution of scripts referenced within usd files by default"                       ON)
option(WITH_SAFETY_OVER_SPEED        "Enable certain checks designed to avoid crashes"                                            ON)
option(WITH_OPENGL                   "Enable OpenGL GFX API and components"                                                       ON)
option(WITH_VULKAN                   "Enable Vulkan GFX API and components"                                                       ON)
if(APPLE) # Only Apple has Metal Support.
  option(WITH_METAL "Enable Metal GFX API and components" ON)
else()
  option(WITH_METAL "Enable Metal GFX API and components" OFF)
endif()
#-------------------------------------------------------------------------------
# Prepare Buildinfo

if(WITH_BUILDINFO)
  find_package(Git)
  if(NOT GIT_FOUND)
    message(WARNING "Git was not found, disabling WITH_BUILDINFO")
    set(WITH_BUILDINFO OFF)
  endif()
endif()
option(WITH_BUILDINFO "Include extra build details (only disable for development & faster builds)" ON)
if(${CMAKE_VERSION} VERSION_LESS 2.8.8)
  # add_library OBJECT arg unsupported
  set(WITH_BUILDINFO OFF)
endif()
set(BUILDINFO_OVERRIDE_DATE    "" CACHE STRING "Use instead of the current date for reproducible builds (empty string disables this option)")
set(BUILDINFO_OVERRIDE_TIME    "" CACHE STRING "Use instead of the current time for reproducible builds (empty string disables this option)")
set(CPACK_OVERRIDE_PACKAGENAME "" CACHE STRING "Use instead of the standard packagename (empty string disables this option)")
mark_as_advanced(CPACK_OVERRIDE_PACKAGENAME)
mark_as_advanced(BUILDINFO_OVERRIDE_DATE)
mark_as_advanced(BUILDINFO_OVERRIDE_TIME)

#-------------------------------------------------------------------------------
# Setup Executable Path

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(WIN32)
    set(CMAKE_INSTALL_PREFIX ${EXECUTABLE_OUTPUT_PATH}/\${BUILD_TYPE} CACHE PATH "default install path" FORCE)
  elseif(APPLE)
    set(CMAKE_INSTALL_PREFIX ${EXECUTABLE_OUTPUT_PATH}/\${BUILD_TYPE} CACHE PATH "default install path" FORCE)
  else()
    if(WITH_INSTALL_PORTABLE)
      set(CMAKE_INSTALL_PREFIX ${EXECUTABLE_OUTPUT_PATH} CACHE PATH "default install path" FORCE)
    endif()
  endif()
endif()

#-----------------------------------------------------------------------------
# Gather Dependencies.

include(Options)
include(ProjectDefaults)
include(Packages)

set(WABI_PYTHON_SHEBANG "${PYTHON_EXECUTABLE}" CACHE STRING "Replacement path for Python #! line.")

include(CXXDefaults)
add_definitions(${_WABI_CXX_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${_WABI_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")

#-----------------------------------------------------------------------------
# COVAHVERSE. The Universe - Descripted.

include(Public)

wabi_covahverse_prologue()

add_subdirectory(wabi)

if(WABI_BUILD_DOCUMENTATION)
  wabi_build_documentation()
endif()

wabi_covahverse_epilogue()

#-----------------------------------------------------------------------------
# COVAH. The Digital Content Creation Platform of the 21st Century.

add_subdirectory(source)

#-----------------------------------------------------------------------------
# Testing

# add_subdirectory(tests)

#-----------------------------------------------------------------------------
# Covah Application ::
#
# For the -- wow this looks like the
# blender build & overall directory
# structure!
#
# Blender's Open Source code taught me how
# to read/write/build C applications.
#
# And likewise, I began to fall in love
# with the overall placement of things
# a bit.
#
# It was the months I spent tinkering around
# in a Blender build that just about taught
# me everything I know.
#
# And I heavily thank you for everything.
#
#          ❤ Open Source
#          ❤ Blender Foundation -- Furby

add_subdirectory(source/creator)

#-----------------------------------------------------------------------------
# CPack for generating packages
include(build_files/cmake/packaging.cmake)

#-----------------------------------------------------------------------------
# Print Final Configuration

if(FIRST_RUN)

  message("                                                          ")
  message("  _ _ _ _____ _____ _____                                 ")
  message(" | | | |  _  | __  |     |                                ")
  message(" | | | |     | __ -|-   -|                                ")
  message(" |_____|__|__|_____|_____|                                ")
  message("     Animation Studios                                    ")
  message("                                                          ")
  message("  COVAH ::: v${COVAH_VERSION_MAJOR}.${COVAH_VERSION_MINOR}")
  message("                                                          ")

  set(_config_msg "\nCOVAH Configuration\n=====================")

  function(info_cfg_option
    _setting
    )

    set(_msg "  - ${_setting}")
    string(LENGTH "${_msg}" _len)
    while("32" GREATER "${_len}")
      set(_msg "${_msg} ")
      math(EXPR _len "${_len} + 1")
    endwhile()

    set(_config_msg "${_config_msg}\n${_msg}${${_setting}}" PARENT_SCOPE)
  endfunction()

  function(info_cfg_text
    _text
    )

    set(_config_msg "${_config_msg}\n\n  ${_text}" PARENT_SCOPE)
  endfunction()

  message(STATUS "C Compiler:   \"${CMAKE_C_COMPILER_ID}\"")
  message(STATUS "C++ Compiler: \"${CMAKE_CXX_COMPILER_ID}\"")

  info_cfg_text("System Options:")
  info_cfg_option(WITH_BUILDINFO)
  info_cfg_option(WITH_LICENSING)
  info_cfg_option(WITH_PIXAR_AR_BETA)
  info_cfg_option(WITH_SAFETY_OVER_SPEED)

  info_cfg_text("Hydra Graphics:")
  info_cfg_option(WITH_VULKAN)
  info_cfg_option(WITH_OPENSUBDIV)
  info_cfg_option(WITH_OPENCOLORIO)
  info_cfg_option(WITH_ARNOLD)
  info_cfg_option(WITH_PRORENDER)
  info_cfg_option(WITH_CYCLES)
  info_cfg_option(WITH_RENDERMAN)
  info_cfg_option(WITH_PHOENIX)
  info_cfg_option(WITH_EMBREE)

  info_cfg_text("Image Formats:")
  info_cfg_option(WITH_OPENIMAGEIO)
  info_cfg_option(WITH_IMAGE_OPENEXR)
  info_cfg_option(WITH_IMAGE_OPENJPEG)
  info_cfg_option(WITH_IMAGE_TIFF)

  info_cfg_text("Dependency Options:")
  info_cfg_option(WITH_USD)
  info_cfg_option(WITH_ALEMBIC)
  info_cfg_option(WITH_TBB)
  info_cfg_option(WITH_OPENVDB)

  info_cfg_text("Python:")
  info_cfg_option(WITH_PYTHON)
  info_cfg_option(WITH_PYTHON_INSTALL)
  info_cfg_option(WITH_PYTHON_INSTALL_NUMPY)
  info_cfg_option(WITH_PYTHON_SECURITY)
  if(APPLE)
    info_cfg_option(WITH_PYTHON_FRAMEWORK)
  endif()

  info_cfg_text("Experimental:")
  info_cfg_option(WITH_XR_OPENXR)

  info_cfg_text("")

  message("${_config_msg}")
endif()

if(0)
  print_all_vars()
endif()
