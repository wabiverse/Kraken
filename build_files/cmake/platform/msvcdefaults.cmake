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

_add_define("WIN32")

option(WITH_WINDOWS_BUNDLE_CRT "Bundle the C runtime for install free distribution." ON)
mark_as_advanced(WITH_WINDOWS_BUNDLE_CRT)

option(WITH_WINDOWS_PDB "Generate a pdb file for client side stacktraces" ON)
mark_as_advanced(WITH_WINDOWS_PDB)

include(build_files/cmake/platform/platform_win32_bundle_crt.cmake)

# X64 ASAN is available and usable on MSVC 16.9 preview 4 and up)
if(WITH_COMPILER_ASAN AND MSVC AND NOT MSVC_CLANG)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.28.29828)
    #set a flag so we don't have to do this comparison all the time
    SET(MSVC_ASAN On)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
    set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} /fsanitize=address")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " /INCREMENTAL:NO")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS_DEBUG " /INCREMENTAL:NO")
  else()
    message("-- ASAN not supported on MSVC ${CMAKE_CXX_COMPILER_VERSION}")
  endif()
endif()

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

# Target Windows 11 SDK.
set(WINDOWS_11_VERSION 10.0.22000.0)

# Our Standard is now :: CXX/WinRT
set(CMAKE_VS_WINRT_BY_DEFAULT ON)
set(CMAKE_SYSTEM_NAME "WindowsStore")
set(CMAKE_SYSTEM_VERSION ${WINDOWS_11_VERSION}) 

# Enable CXX/WinRT (20++) hybrid features.
# Our Windows Standard. For the long haul.
set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /ZW")


# Temporary, until they fixup the installation.
set(WINDOWS_11_STORE
  "C:/Program Files (x86)/Windows Kits/10/UnionMetadata/${WINDOWS_11_VERSION}"
)
set(WINDOWS_11_PLATFORM
  "C:/Program Files/Microsoft Visual Studio/2022/Preview/Common7/IDE/VC/vcpackages"
)
list(APPEND _WABI_CXX_FLAGS /AI"${WINDOWS_11_STORE}")
list(APPEND _WABI_CXX_FLAGS /AI"${WINDOWS_11_PLATFORM}")

# Enable exception handling.
if(KRAKEN_RELEASE_MODE)
    set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} ${_WABI_CXX_FLAGS_RELEASE} /EHsc")
else()
    set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} ${_WABI_CXX_FLAGS_DEBUG} /EHsc")
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

set(_WABI_CXX_FLAGS ${_WABI_CXX_FLAGS} ${_WABI_CXX_WARNING_FLAGS})

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
# if (NOT Boost_USE_STATIC_LIBS)
_add_define("BOOST_ALL_DYN_LINK")
# endif()

# Need half::_toFloat and half::_eLut.
_add_define("OPENEXR_DLL")
_add_define("IMATH_DLL")

# Need M_PI_2 and other math defines
_add_define("_USE_MATH_DEFINES")

# These files require /bigobj compiler flag
#   Vt/arrayPyBuffer.cpp
#   Usd/crateFile.cpp
#   Usd/stage.cpp
# Until we can set the flag on a per file basis, we'll have to enable it
# for all translation units.
set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /bigobj")

# Enable PDB generation.
set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} /Zi")

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

# Ignore LNK4221.  This happens when making an archive with a object file
# with no symbols in it.  We do this a lot because of a pattern of having
# a C++ source file for many header-only facilities, e.g. tf/bitUtils.cpp.
string(APPEND PLATFORM_LINKFLAGS " /SUBSYSTEM:CONSOLE /STACK:2097152")
set(PLATFORM_LINKFLAGS_RELEASE "/NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")
string(APPEND PLATFORM_LINKFLAGS_DEBUG " /IGNORE:4099 /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcmtd.lib")

# Ignore meaningless for us linker warnings.
string(APPEND PLATFORM_LINKFLAGS " /ignore:4049 /ignore:4217 /ignore:4221")
set(PLATFORM_LINKFLAGS_RELEASE "${PLATFORM_LINKFLAGS} ${PDB_INFO_OVERRIDE_LINKER_FLAGS}")
string(APPEND CMAKE_STATIC_LINKER_FLAGS " /ignore:4221")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  string(PREPEND PLATFORM_LINKFLAGS "/MACHINE:X64 ")
else()
  string(PREPEND PLATFORM_LINKFLAGS "/MACHINE:IX86 /LARGEADDRESSAWARE ")
endif()

# When Visual Studio IDE runs the project for testing
# it needs to know which project to execute.
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT kraken)