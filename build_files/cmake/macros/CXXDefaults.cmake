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
include(CXXHelpers)
include(Options)

# --- Require (C++20) ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(UNIX AND NOT APPLE)
    include(gccdefaults)
elseif(APPLE)
    include(clangdefaults)
    include(xcodedefaults)
elseif(WIN32)
    include(msvcdefaults)
endif()

_add_define(GL_GLEXT_PROTOTYPES)
_add_define(GLX_GLXEXT_PROTOTYPES)

# Python bindings for tf require this define.
_add_define(BOOST_PYTHON_NO_PY_SIGNATURES)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  _add_define(BUILD_OPTLEVEL_DEV)
endif()

# Set plugin path environment variable name
set(WABI_PLUGINPATH_NAME WABI_PLUGINPATH_NAME)
if (WABI_OVERRIDE_PLUGINPATH_NAME)
  set(WABI_PLUGINPATH_NAME ${WABI_OVERRIDE_PLUGINPATH_NAME})
  _add_define("WABI_PLUGINPATH_NAME=${WABI_PLUGINPATH_NAME}")
endif()

# Set namespace configuration.
if(WABI_ENABLE_NAMESPACES)
    set(WABI_USE_NAMESPACES "1")

    if(WABI_SET_EXTERNAL_NAMESPACE)
        set(WABI_EXTERNAL_NAMESPACE ${WABI_SET_EXTERNAL_NAMESPACE})
    else()
        set(WABI_EXTERNAL_NAMESPACE "wabi")
    endif()

    if(WABI_SET_INTERNAL_NAMESPACE)
        set(WABI_INTERNAL_NAMESPACE ${WABI_SET_INTERNAL_NAMESPACE})
    else()
        set(WABI_INTERNAL_NAMESPACE "WABImaelstrom")
    endif()
else()
    set(WABI_USE_NAMESPACES "0")
    message(STATUS "C++ namespaces disabled.")
endif()

set(CMAKE_MESSAGE_LOG_LEVEL "WARNING" CACHE STRING "Remove excessive cmake logging." FORCE)