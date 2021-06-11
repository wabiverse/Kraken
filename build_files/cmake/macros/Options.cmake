# 
#  Copyright 2021 Pixar. All Rights Reserved.
# 
#  Portions of this file are derived from original work by Pixar
#  distributed with Universal Scene Description, a project of the
#  Academy Software Foundation (ASWF). https://www.aswf.io/
# 
#  Licensed under the Apache License, Version 2.0 (the "Apache License")
#  with the following modification; you may not use this file except in
#  compliance with the Apache License and the following modification:
#  Section 6. Trademarks. is deleted and replaced with:
# 
#  6. Trademarks. This License does not grant permission to use the trade
#     names, trademarks, service marks, or product names of the Licensor
#     and its affiliates, except as required to comply with Section 4(c)
#     of the License and to reproduce the content of the NOTICE file.
#
#  You may obtain a copy of the Apache License at:
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the Apache License with the above modification is
#  distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
#  ANY KIND, either express or implied. See the Apache License for the
#  specific language governing permissions and limitations under the
#  Apache License.
#
#  Modifications copyright (C) 2020-2021 Wabi.
#
# -----------------------------------------------------------------------------------------------------------------------------
option(WABI_STRICT_BUILD_MODE                   "Turn on additional warnings. Enforce all warnings as errors."            OFF)
option(WABI_VALIDATE_GENERATED_CODE             "Validate script generated code"                                           ON)
option(WABI_HEADLESS_TEST_MODE                  "Disallow GUI based tests, useful for running under headless CI systems." OFF)
option(WABI_BUILD_TESTS                         "Build tests"                                                             OFF)
option(WABI_BUILD_EXAMPLES                      "Build examples"                                                          OFF)
option(WABI_BUILD_TUTORIALS                     "Build tutorials"                                                         OFF)
option(WABI_BUILD_USD_TOOLS                     "Build commandline tools"                                                  ON)
option(WABI_BUILD_IMAGING                       "Build imaging components"                                                 ON)
option(WABI_BUILD_USD_IMAGING                   "Build USD imaging components"                                             ON)
option(WABI_BUILD_DOCUMENTATION                 "Generate doxygen documentation"                                          OFF)
option(WABI_ENABLE_PYTHON_SUPPORT               "Enable Python based components for USD"                                   ON)
option(WABI_ENABLE_NAMESPACES                   "Enable C++ namespaces."                                                   ON)
# -----------------------------------------------------------------------------------------------------------------------------

# Precompiled headers are a win on Windows, not on gcc.
set(wabi_enable_pch "OFF")
if(MSVC)
    set(wabi_enable_pch "ON")
endif()
option(WABI_ENABLE_PRECOMPILED_HEADERS "Enable precompiled headers." "${wabi_enable_pch}")
set(WABI_PRECOMPILED_HEADER_NAME "pch.h"
    CACHE
    STRING
    "Default name of precompiled header files"
)

set(WABI_INSTALL_LOCATION ""
    CACHE
    STRING
    "Intended final location for plugin resource files."
)

set(WABI_OVERRIDE_PLUGINPATH_NAME ""
    CACHE
    STRING
    "Name of the environment variable that will be used to get plugin paths."
)

set(WABI_ALL_LIBS ""
    CACHE
    INTERNAL
    "Aggregation of all built libraries."
)
set(WABI_STATIC_LIBS ""
    CACHE
    INTERNAL
    "Aggregation of all built explicitly static libraries."
)
set(WABI_CORE_LIBS ""
    CACHE
    INTERNAL
    "Aggregation of all built core libraries."
)
set(WABI_OBJECT_LIBS ""
    CACHE
    INTERNAL
    "Aggregation of all core libraries built as OBJECT libraries."
)

set(WABI_LIB_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX}
    CACHE
    STRING
    "Prefix for build library name"
)

set(WABI_MONOLITHIC_IMPORT "" CACHE STRING   "Path to cmake file that imports a covahverse target")
set(WABI_EXTRA_PLUGINS     "" CACHE INTERNAL "Aggregation of extra plugin directories containing a plugInfo.json.")

# Resolve options that depend on one another so that subsequent .cmake scripts
# all have the final value for these options.
if (${WABI_BUILD_USD_IMAGING} AND NOT ${WABI_BUILD_IMAGING})
  message(STATUS "Setting WABI_BUILD_USD_IMAGING=OFF because WABI_BUILD_IMAGING=OFF")
  set(WABI_BUILD_USD_IMAGING "OFF" CACHE BOOL "" FORCE)
endif()

if(${WITH_OPENGL} OR ${WITH_METAL} OR ${WITH_VULKAN})
  set(WITH_GPU_SUPPORT "ON")
else()
  set(WITH_GPU_SUPPORT "OFF")
endif()

if(${WITH_PIXAR_USDVIEW})
  if(NOT ${WABI_BUILD_USD_IMAGING})
    message(STATUS "Setting WITH_PIXAR_USDVIEW=OFF because WABI_BUILD_USD_IMAGING=OFF")
    set(WITH_PIXAR_USDVIEW "OFF" CACHE BOOL "" FORCE)
  elseif (NOT ${WABI_ENABLE_PYTHON_SUPPORT})
    message(STATUS "Setting WITH_PIXAR_USDVIEW=OFF because WABI_ENABLE_PYTHON_SUPPORT=OFF")
    set(WITH_PIXAR_USDVIEW "OFF" CACHE BOOL "" FORCE)
  elseif (NOT ${WITH_GPU_SUPPORT})
    message(STATUS "Setting WITH_PIXAR_USDVIEW=OFF because WITH_GPU_SUPPORT=OFF")
    set(WITH_PIXAR_USDVIEW "OFF" CACHE BOOL "" FORCE)
  endif()
endif()

if(${WITH_EMBREE})
  if (NOT ${WABI_BUILD_IMAGING})
    message(STATUS "Setting WITH_EMBREE=OFF because WABI_BUILD_IMAGING=OFF")
    set(WITH_EMBREE "OFF" CACHE BOOL "" FORCE)
  elseif (NOT ${WITH_GPU_SUPPORT})
    message(STATUS "Setting WITH_EMBREE=OFF because WITH_GPU_SUPPORT=OFF")
    set(WITH_EMBREE "OFF" CACHE BOOL "" FORCE)
  endif()
endif()

if(${WITH_RENDERMAN})
  if(NOT ${WABI_BUILD_IMAGING})
    message(STATUS "Setting WITH_RENDERMAN=OFF because WABI_BUILD_IMAGING=OFF")
    set(WITH_RENDERMAN "OFF" CACHE BOOL "" FORCE)
  endif()
endif()

# Make sure the MaterialX Plugin is built when enabling MaterialX Imaging
if(${WITH_MATERIALX})
  set(WITH_MATERIALX "ON" CACHE BOOL "" FORCE)
endif()

if(WITH_PYTHON)
  add_definitions(-DWITH_PYTHON)
endif()

if(WITH_SAFETY_OVER_SPEED)
  add_definitions(-DWITH_SAFETY_OVER_SPEED)
endif()

if(WITH_PIXAR_AR_BETA)
  add_definitions(-DWITH_PIXAR_AR_BETA)
endif()

if(WITH_PIXAR_USDVIEW)
  add_definitions(-DWITH_PIXAR_USDVIEW)
endif()