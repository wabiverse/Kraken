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

# -----------------------------------------------------------------------------
# Build options specific for Windows

option(WITH_WINDOWS_BUNDLE_CRT "Bundle the C runtime for install free distribution." ON)
mark_as_advanced(WITH_WINDOWS_BUNDLE_CRT)

option(WITH_WINDOWS_PDB "Generate a pdb file for client side stacktraces" ON)
mark_as_advanced(WITH_WINDOWS_PDB)

option(WITH_WINDOWS_STRIPPED_PDB "Use a stripped PDB file" On)
mark_as_advanced(WITH_WINDOWS_STRIPPED_PDB)

include(build_files/cmake/platform/nuget_packages.cmake)
include(build_files/cmake/platform/microsoft_package.cmake)

# -----------------------------------------------------------------------------
# Setup Compiler Flags

if(MSVC_ASAN)
  set(SYMBOL_FORMAT /Z7)
  set(SYMBOL_FORMAT_RELEASE /Z7)
else()
  set(SYMBOL_FORMAT /ZI)
  set(SYMBOL_FORMAT_RELEASE /Zi)
endif()

if(WITH_WINDOWS_PDB)
  set(PDB_INFO_OVERRIDE_FLAGS "${SYMBOL_FORMAT_RELEASE}")
  set(PDB_INFO_OVERRIDE_LINKER_FLAGS "/DEBUG /OPT:REF /OPT:ICF /INCREMENTAL:NO")
endif()

set(_WABI_CXX_FLAGS_DEBUG "${_WABI_CXX_FLAGS_DEBUG} /MDd ${SYMBOL_FORMAT}")
set(_WABI_C_FLAGS_DEBUG   "${_WABI_C_FLAGS_DEBUG} /MDd ${SYMBOL_FORMAT}")
set(_WABI_CXX_FLAGS_RELEASE    "${_WABI_CXX_FLAGS_RELEASE} /MD ${PDB_INFO_OVERRIDE_FLAGS}")
set(_WABI_C_FLAGS_RELEASE      "${_WABI_C_FLAGS_RELEASE} /MD ${PDB_INFO_OVERRIDE_FLAGS}")
set(_WABI_CXX_FLAGS_MINSIZEREL "${_WABI_CXX_FLAGS_MINSIZEREL} /MD ${PDB_INFO_OVERRIDE_FLAGS}")
set(_WABI_C_FLAGS_MINSIZEREL   "${_WABI_C_FLAGS_MINSIZEREL} /MD ${PDB_INFO_OVERRIDE_FLAGS}")
set(_WABI_CXX_FLAGS_RELWITHDEBINFO "${_WABI_CXX_FLAGS_RELWITHDEBINFO} /MD ${SYMBOL_FORMAT_RELEASE}")
set(_WABI_C_FLAGS_RELWITHDEBINFO   "${_WABI_C_FLAGS_RELWITHDEBINFO} /MD ${SYMBOL_FORMAT_RELEASE}")
unset(SYMBOL_FORMAT)
unset(SYMBOL_FORMAT_RELEASE)

# You should be on MSVC 2022 (>=17)
set(COMPILER_VERSION "17")

# Target Windows 11 SDK.
set(WINDOWS_SDK_VERSION "10.0.22000.0")

# Disable C++/CX
set(CMAKE_VS_WINRT_BY_DEFAULT OFF)

# Set the CMAKE build system to target UWP
# this effectively enables WinRT compilation
set(CMAKE_SYSTEM_NAME WindowsStore)
set(CMAKE_SYSTEM_VERSION ${WINDOWS_SDK_VERSION})

# Things are in preview so we need to add this for now.
set(WINDOWS_SDK "Windows.Universal")
set(VC_LIBRARIES_VERSION "17.0")

# Enable CXX/WinRT (20++) hybrid features.
# Our Windows Standard. For the long haul.
set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /EHsc")

# Temporary, until they fixup the installation.
file(TO_CMAKE_PATH "C:/Program Files (x86)/Windows Kits/10/UnionMetadata/${WINDOWS_SDK_VERSION}"
  WINDOWS_11_STORE
)
file(TO_CMAKE_PATH "C:/Program Files/Microsoft Visual Studio/2022/Preview/Common7/IDE/VC/vcpackages"
  WINDOWS_11_PLATFORM
)

set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /AI\"${CMAKE_BINARY_DIR}/bin/Release\"")

# Enable exception handling.
if(KRAKEN_RELEASE_MODE)
    set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} ${_WABI_CXX_FLAGS_RELEASE}")
else()
    set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} ${_WABI_CXX_FLAGS_DEBUG}")
endif()

# Standards compliant.
set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /Zc:rvalueCast /Zc:strictStrings")

# The /Zc:inline option strips out the "arch_ctor_<name>" symbols used for
# library initialization by ARCH_CONSTRUCTOR starting in Visual Studio 2019,
# causing release builds to fail. Disable the option for this and later
# versions.
#
# TODO: Test this on Visual Studio 2022 and Beyond.
#
# For more details, see:
# https://developercommunity.visualstudio.com/content/problem/914943/zcinline-removes-extern-symbols-inside-anonymous-n.html
if (MSVC_VERSION GREATER_EQUAL 1920)
    set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /Zc:inline-")
else()
    set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /Zc:inline")
endif()

# Turn on all but informational warnings.
set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /W3")

# Warnings are errors in strict build mode.
if (${WABI_STRICT_BUILD_MODE})
    set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /WX")
endif()

# These files require /bigobj compiler flag
#   Vt/arrayPyBuffer.cpp
#   Usd/crateFile.cpp
#   Usd/stage.cpp
# Until we can set the flag on a per file basis, we'll have to enable it
# for all translation units.
set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /bigobj")

# Enable multiprocessor builds.
set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /MP")
set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /Gm-")

# Enable 'Just My Code' debugging, allowing MSVC to step over system, framework,
# libraries, and other non-user calls, and to collapse those calls in the call stack
# window. The /JMC compiler option is available starting in Visual Studio 2017 version
# 15.8 (1915) and up.
if(MSVC_VERSION GREATER 1914 AND NOT MSVC_CLANG)
  set(_WABI_CXX_FLAGS_DEBUG "${_WABI_CXX_FLAGS_DEBUG} /JMC")
endif()

# -----------------------------------------------------------------------------
# Setup Disabled Warnings

# Don't treat these warnings as errors.
_disable_warning("4146")
_disable_warning("4996")

# truncation from 'double' to 'float' due to matrix and vector classes in `Gf`
_disable_warning("4244")
_disable_warning("4305")

# conversion from size_t to int. While we don't want this enabled
# it's in the Python headers. So all the Python wrap code is affected.
_disable_warning("4267")

# no definition for inline function
# this affects Glf only
_disable_warning("4506")

# 'typedef ': ignored on left of '' when no variable is declared
# XXX:figure out why we need this
_disable_warning("4091")

# c:\python27\include\pymath.h(22): warning C4273: 'round': inconsistent dll linkage
# XXX:figure out real fix
_disable_warning("4273")

# qualifier applied to function type has no meaning; ignored
# tbb/parallel_for_each.h
_disable_warning("4180")

# '<<': result of 32-bit shift implicitly converted to 64 bits
# tbb/enumerable_thread_specific.h
_disable_warning("4334")

# Disable warnings for libraries like Vulkan which do not supply
# debugging PDB files.
_disable_warning("4099")

# -----------------------------------------------------------------------------
# Setup Preprocessor Definitions

# A safe check to detect whether or not we are building
# on windows, however, prefer to use (ARCH_OS_WINDOWS)
# from wabi/base/arch/defines.h where possible.
_add_define("WIN32")

# Disable warning C4996 regarding fopen(), strcpy(), etc.
_add_define("_CRT_SECURE_NO_WARNINGS")

# Disable warning C4996 regarding unchecked iterators for std::transform,
# std::copy, std::equal, et al.
_add_define("_SCL_SECURE_NO_WARNINGS")

# Make sure WinDef.h does not define min and max macros which
# will conflict with std::min() and std::max().
_add_define("NOMINMAX")

# Needed to prevent YY files trying to include unistd.h
# (which doesn't exist on Windows)
_add_define("YY_NO_UNISTD_H")

# Forces all libraries that have separate source to be linked as
# DLL's rather than static libraries on Microsoft Windows, unless
# explicitly told otherwise.
_add_define("BOOST_ALL_DYN_LINK")

# Need half::_toFloat and half::_eLut.
_add_define("OPENEXR_DLL")
_add_define("IMATH_DLL")

# Need M_PI_2 and other math defines
_add_define("_USE_MATH_DEFINES")

# These need to be set or else we get errors
# from these warnings when compiling with the
# Windows Runtime (C++/CX) "/ZW" compiler
# flag.
_add_define("_SCL_SECURE_NO_WARNINGS")
_add_define("_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING")
_add_define("_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS")

# Per Microsoft: You should always call the Unicode versions.
# Many world languages require Unicode. If you use ANSI strings,
# it will be impossible to localize your application. The ANSI
# versions are also less efficient, because the operating system
# must convert the ANSI strings to Unicode at run time.
# !! Be careful: Some headers use the preprocessor symbol UNICODE,
# others use _UNICODE with an underscore prefix. Always define both
# symbols. Visual C++ sets them both by default when you create a
# new project.
_add_define("UNICODE")
_add_define("_UNICODE")

# -----------------------------------------------------------------------------
# Setup Linker Flags

string(APPEND PLATFORM_LINKFLAGS " /SUBSYSTEM:WINDOWS /WINMD /STACK:2097152")
string(APPEND PLATFORM_LINKFLAGS_DEBUG " /IGNORE:4099")

# Ignore meaningless for us linker warnings.
string(APPEND PLATFORM_LINKFLAGS " /ignore:4049 /ignore:4217 /ignore:4221")
set(PLATFORM_LINKFLAGS_RELEASE "${PLATFORM_LINKFLAGS} ${PDB_INFO_OVERRIDE_LINKER_FLAGS}")
string(APPEND CMAKE_STATIC_LINKER_FLAGS " /ignore:4221")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  string(PREPEND PLATFORM_LINKFLAGS "/MACHINE:X64 ")
else()
  string(PREPEND PLATFORM_LINKFLAGS "/MACHINE:IX86 /LARGEADDRESSAWARE ")
endif()

# -----------------------------------------------------------------------------
# Setup Default Project to open Visual Studio IDE with (Optional)

# When opening the cmake generated Visual Studio Solution
# within Visual Studio IDE - the IDE needs a default vsproj
# to open by default.
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT kraken)