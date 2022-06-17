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
if(APPLE)
    set(OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for OSX")
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/bin")
    set(TARGETDIR_VER ${CMAKE_INSTALL_PREFIX}/Kraken.app/Contents/Resources/${KRAKEN_VERSION})
    set(CMAKE_DYLIB_INSTALL_NAME_DIR "${TARGETDIR_VER}/lib" CACHE STRING "install_name path for dylib.")
    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${TARGETDIR_VER}/lib" isSystemDir)
endif()

# Allow local includes from source directory.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on folder usage
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# if (WABI_BUILD_TESTS)
#     # Enable CTest
#     enable_testing()
# endif()
