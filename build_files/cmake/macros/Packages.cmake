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

# Save the current value of BUILD_SHARED_LIBS and restore it at
# the end of this file, since some of the Find* modules invoked
# below may wind up stomping over this value.

set(build_shared_libs "${BUILD_SHARED_LIBS}")

#-------------------------------------------------------------------------------------------------------------------------------------------
# Set directory path to precompiled libraries

if(WIN32)
  if(MSVC_VERSION GREATER_EQUAL 1929)
    # MSVC 2022+
    set(LIBPATH ${CMAKE_SOURCE_DIR}/../lib/win64_vc17)
  elseif(MSVC_VERSION GREATER_EQUAL 1920)
    # MSVC 2019+
    set(LIBPATH ${CMAKE_SOURCE_DIR}/../lib/win64_vc16)
  else()
    # MSVC XXXX < 2019
    # A friendly reminder that this is
    # not officially supported. Time to
    # update compilers my friend.
    set(LIBPATH ${CMAKE_SOURCE_DIR}/../lib/win64_vc15)
  endif()
  set(LIB_OBJ_EXT "lib")
elseif(UNIX AND NOT APPLE)
  set(LIBPATH ${CMAKE_SOURCE_DIR}/../lib/linux_centos7_x86_64)
  set(LIB_OBJ_EXT "so")
elseif(APPLE)
  set(LIBPATH ${CMAKE_SOURCE_DIR}/../lib/apple_darwin_x86_64)
  set(LIB_OBJ_EXT "dylib")
endif()

# ! Important
# Convert the relative path to an absolute path.
string(REPLACE "kraken/../" "" LIBDIR "${LIBPATH}")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")

# Note: We place Imath into OpenEXR installation for
# our precompiled libraries for simplicity. However, 
# if using find_package() for system libraries (UNIX)
# Imath is a wholly sperate package now as of v3.0.
#
# For those unfamiliar, Imath holds all our important
# data types such as half, vector, matrix, bbox, color,
# etc.
#
# Further investigation is necessary to determine Pixar's
# Gf types which hold equivalent data types to these data
# types provided by Industrial Light & Magic.
#
# However, within the scope of a Pixar USD build, the Pixar
# data types may hold better compliancy with other DCC programs
# scripts & projects.
#
# On the contrary, Industrial Light & Magic's data types may
# have better studio adoption, and therefore may be better in
# a production pipeline.
#
# Perhaps the two are equivalent interpretations of each other?
# Probably not likely, or prone to weird errors...
#
# TODO: Allow for a build option that favors ILM types over Pixar
# types, if request for this option gains popularity. Though, it
# may be complicated to embed these ILM types into Python as
# natively as the Pixar Gf types -- though, v3.0 of Imath seems
# to offer substantial python support, and thus, may not be a
# problem.
#
# References:
# ====================================================== PIXAR =====
# GF: https://graphics.pixar.com/usd/docs/api/gf_page_front.html
#
# ======================================================= IL&M =====
# OPENEXR: https://github.com/AcademySoftwareFoundation/openexr
# IMATH:   https://github.com/AcademySoftwareFoundation/Imath
#
# And somewhere within Pixar's Graphics Foundations; ILM's half,
# float, and other's are used. To which extent needs investigation.
#
# TLDR :: What's the difference between PIXAR vs IL&M types?
#      :: INVESTIGATE ME!!
#

if(WIN32)
  set(FREETYPE_DIR          ${LIBDIR}/freetype)
  set(OPENEXR_LOCATION      ${LIBDIR}/openexr)
  set(OCIO_LOCATION         ${LIBDIR}/opencolorio)
  set(OIIO_LOCATION         ${LIBDIR}/OpenImageIO)
  set(ZLIB_INCLUDE_DIR      ${LIBDIR}/zlib/include)
  set(ZLIB_LIBRARY          ${LIBDIR}/zlib/lib/zlibstatic.lib)
  set(OPENSUBDIV_ROOT_DIR   ${LIBDIR}/opensubdiv)
  set(OpenImageDenoise_ROOT ${LIBDIR}/oidn)
  set(PTEX_LOCATION         ${LIBDIR}/ptex)
  set(HDF5_ROOT             ${LIBDIR}/hdf5)
  set(OPENVDB_LOCATION      ${LIBDIR}/openvdb)
  set(ALEMBIC_DIR           ${LIBDIR}/alembic)
  set(OSL_LOCATION          ${LIBDIR}/osl)
  set(EMBREE_LOCATION       ${LIBDIR}/embree)
  set(MATERIALX_ROOT        ${LIBDIR}/MaterialX)
  set(MATERIALX_DATA_ROOT   ${LIBDIR}/MaterialX/libraries)
  set(TBB_ROOT_DIR          ${LIBDIR}/tbb)

  set(OPENEXR_INCLUDE_DIR "${LIBDIR}/openexr/include")
  set(IMATH_INCLUDE_DIR "${LIBDIR}/openexr/include/Imath")
  set(ILMBASE_INCLUDES "${LIBDIR}/openexr/include")
  list(APPEND OPENEXR_LIBRARIES
    "${LIBDIR}/openexr/lib/Iex-3_0.lib"
    "${LIBDIR}/openexr/lib/IlmThread-3_0.lib"
    "${LIBDIR}/openexr/lib/Imath-3_0.lib"
    "${LIBDIR}/openexr/lib/OpenEXR-3_0.lib"
    "${LIBDIR}/openexr/lib/OpenEXRUtil-3_0.lib")
else()
  list(APPEND CMAKE_PREFIX_PATH "${LIBDIR}")
  SUBDIRLIST(subdir_list "${LIBDIR}/lib/cmake")
  foreach(subdir ${subdir_list})
    list(APPEND CMAKE_PREFIX_PATH "${LIBDIR}/lib/cmake/${subdir}")
  endforeach()
endif()
#-------------------------------------------------------------------------------------------------------------------------------------------
# Find Threads

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
set(WABI_THREAD_LIBS "${CMAKE_THREAD_LIBS_INIT}")

if(WIN32)
  set(PTHREADS_INCLUDE_DIR ${LIBDIR}/pthreads/include)
  set(PTHREADS_LIBRARY ${LIBDIR}/pthreads/lib/pthreadVC3.lib)
endif()

#-------------------------------------------------------------------------------------------------------------------------------------------
# Find Python

if(WIN32)
  set(FOUND_PYTHON ON)
  set(Python_VERSION_MAJOR "3")
  set(Python_VERSION_MINOR "9")
  set(python_version_nodot "${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
  set(PYTHON_INCLUDE_DIR ${LIBDIR}/python/${python_version_nodot}/include)
  set(PYTHON_LIBPATH ${LIBDIR}/python/${python_version_nodot}/lib)

  if(KRAKEN_RELEASE_MODE)
    set(PYTHON_EXECUTABLE ${LIBDIR}/python/${python_version_nodot}/bin/python.exe)
    set(PYTHON_LIBRARIES ${LIBDIR}/python/${python_version_nodot}/libs/python${python_version_nodot}.lib)
  else()
    set(PYTHON_EXECUTABLE ${LIBDIR}/python/${python_version_nodot}/bin/python.exe)
    set(PYTHON_LIBRARIES
      ${LIBDIR}/python/${python_version_nodot}/libs/python${python_version_nodot}_d.lib
      ${LIBDIR}/python/${python_version_nodot}/libs/python${python_version_nodot}.lib  
    )
  endif()

  link_libraries(
    $<$<CONFIG:Debug>:${LIBDIR}/python/${python_version_nodot}/libs/python${python_version_nodot}_d.lib>
    $<$<CONFIG:Release>:${LIBDIR}/python/${python_version_nodot}/libs/python${python_version_nodot}.lib>
  )
else()
  find_package(Python 3.9 COMPONENTS Interpreter Development REQUIRED)
  set(PYTHON_INCLUDE_DIR ${Python_INCLUDE_DIRS})
  set(PYTHON_LIBPATH ${Python_LIBRARY_DIRS})
  set(PYTHON_LIBRARIES ${Python_LIBRARIES})
  set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
endif()

#-------------------------------------------------------------------------------------------------------------------------------------------
# Find Freetype

if(UNIX)
  find_package(Freetype REQUIRED)
elseif(WIN32)
  set(FREETYPE_INCLUDE_DIRS ${FREETYPE_DIR}/include/freetype2)
  set(FREETYPE_LIBRARY ${FREETYPE_DIR}/lib/freetype2ST.lib)
endif()

#-------------------------------------------------------------------------------------------------------------------------------------------
# Configure GFX APIs

# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx OpenGL xxxxx
if(WITH_OPENGL)
  add_definitions(-DWITH_OPENGL)
  find_package(OpenGL REQUIRED)
  kraken_include_dirs_sys("${OPENGL_INCLUDE_DIR}")
endif()

# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx Metal xxxxx
if(WITH_METAL)
  add_definitions(-DWITH_METAL)
endif()

# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx Vulkan xxxxx
if(WITH_VULKAN)

  find_package(Vulkan REQUIRED)

  list(APPEND VULKAN_LIBS
    Vulkan::Vulkan
  )

  # Find the OS specific libs we need
  if(APPLE)
    find_library(MVK_LIBRARIES
      NAMES MoltenVK
      PATHS ${VULKAN_SDK_DIR}/lib
    )
    list(APPEND VULKAN_LIBS ${MVK_LIBRARIES})
  elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    list(APPEND VULKAN_LIBS ${X11_LIBRARIES})
  elseif(WIN32)
    file(TO_CMAKE_PATH "$ENV{VULKAN_SDK}" VULKAN_DIR_PATH)
    if(KRAKEN_RELEASE_MODE)
      list(APPEND VULKAN_LIBS
        ${VULKAN_DIR_PATH}/Lib/glslang.lib
        ${VULKAN_DIR_PATH}/Lib/OGLCompiler.lib
        ${VULKAN_DIR_PATH}/Lib/GenericCodeGen.lib
        ${VULKAN_DIR_PATH}/Lib/MachineIndependent.lib
        ${VULKAN_DIR_PATH}/Lib/OSDependent.lib
        ${VULKAN_DIR_PATH}/Lib/shaderc.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRV.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRV-Tools.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRV-Tools-link.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRV-Tools-opt.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRV-Tools-reduce.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRV-Tools-shared.lib
      )
    else()
      list(APPEND VULKAN_LIBS
        ${VULKAN_DIR_PATH}/Lib/glslangd.lib
        ${VULKAN_DIR_PATH}/Lib/OGLCompilerd.lib
        ${VULKAN_DIR_PATH}/Lib/GenericCodeGend.lib
        ${VULKAN_DIR_PATH}/Lib/MachineIndependentd.lib
        ${VULKAN_DIR_PATH}/Lib/OSDependentd.lib
        ${VULKAN_DIR_PATH}/Lib/shadercd.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRVd.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRV-Toolsd.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRV-Tools-linkd.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRV-Tools-optd.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRV-Tools-reduced.lib
        ${VULKAN_DIR_PATH}/Lib/SPIRV-Tools-sharedd.lib
      )
    endif()
  endif()

  add_definitions(-DWITH_VULKAN)

  set(VULKAN_INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS})
  list(APPEND VULKAN_LIBRARIES
    ${Vulkan_LIBRARIES}
    ${VULKAN_LIBS}
  )

  kraken_include_dirs_sys("${VULKAN_INCLUDE_DIRS}")

else()
  message(FATAL_ERROR "Vulkan Installation not valid")
endif()

if(WITH_DIRECTX)
  list(APPEND DIRECTX_LIBS
    d3d12
    d3dcompiler
    dxgi
  )
  add_definitions(-DWITH_DIRECTX)
endif()

if(UNIX AND NOT APPLE)
  find_package(X11)
endif()

if(UNIX AND NOT APPLE)
  set(SDL2_INCLUDE_DIR /usr/include/SDL2)
  list(APPEND SDL2_LIBRARIES
    /usr/lib64/libSDL2.so
    /usr/lib64/libSDL2_image.so
    /usr/lib64/libSDL2_ttf.so)
endif()

#-------------------------------------------------------------------------------------------------------------------------------------------
# Find BOOST

if(WIN32)
  set(LIB_OBJ_EXT "lib")
  set(BOOST_VERSION_SCORE "1_78")
  if(KRAKEN_RELEASE_MODE)
    set(BOOST_LIBRARY_SUFFIX "vc143-mt-x64-1_78")
  else()
    set(BOOST_LIBRARY_SUFFIX "vc143-mt-gd-x64-1_78")
  endif()
  # set(Boost_USE_STATIC_RUNTIME ON) # prefix lib
  # set(Boost_USE_MULTITHREADED ON) # suffix -mt
  # set(Boost_USE_STATIC_LIBS ON) # suffix -s
  set(BOOST_ROOT "${LIBDIR}/boost")
  set(Boost_INCLUDE_DIR "${LIBDIR}/boost/include/boost-1_78")
  find_package(Boost REQUIRED)
  set(boost_version_string "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")

  set(Boost_INCLUDE_DIRS           ${LIBDIR}/boost/include/boost-${BOOST_VERSION_SCORE})
  set(Boost_ATOMIC_LIBRARY         ${LIBDIR}/boost/lib/boost_atomic-${BOOST_LIBRARY_SUFFIX}.lib)
  set(Boost_CHRONO_LIBRARY         ${LIBDIR}/boost/lib/boost_chrono-${BOOST_LIBRARY_SUFFIX}.lib)
  set(Boost_DATETIME_LIBRARY       ${LIBDIR}/boost/lib/boost_date_time-${BOOST_LIBRARY_SUFFIX}.lib)
  set(Boost_FILESYSTEM_LIBRARY     ${LIBDIR}/boost/lib/boost_filesystem-${BOOST_LIBRARY_SUFFIX}.lib)
  set(Boost_IOSTREAMS_LIBRARY      ${LIBDIR}/boost/lib/boost_iostreams-${BOOST_LIBRARY_SUFFIX}.lib)
  set(Boost_NUMPY_LIBRARY          ${LIBDIR}/boost/lib/boost_numpy${python_version_nodot}-${BOOST_LIBRARY_SUFFIX}.lib)
  set(Boost_PYTHON_LIBRARY         ${LIBDIR}/boost/lib/boost_python${python_version_nodot}-${BOOST_LIBRARY_SUFFIX}.lib)
  set(Boost_PROGRAMOPTIONS_LIBRARY ${LIBDIR}/boost/lib/boost_program_options-${BOOST_LIBRARY_SUFFIX}.lib)
  set(Boost_REGEX_LIBRARY          ${LIBDIR}/boost/lib/boost_regex-${BOOST_LIBRARY_SUFFIX}.lib)
  set(Boost_SYSTEM_LIBRARY         ${LIBDIR}/boost/lib/boost_system-${BOOST_LIBRARY_SUFFIX}.lib)
  set(Boost_THREAD_LIBRARY         ${LIBDIR}/boost/lib/boost_thread-${BOOST_LIBRARY_SUFFIX}.lib)

elseif(UNIX)
  add_definitions(-DBoost_NO_BOOST_CMAKE=1)
  # set(BOOST_ROOT "${LIBDIR}")
  find_package(Boost REQUIRED
               COMPONENTS
                 atomic
                 program_options 
                 date_time 
                 filesystem 
                 system
                 regex
                 python39
                 thread
                 iostreams
  )
  set(Boost_ATOMIC_LIBRARY         Boost::atomic)
  set(Boost_DATETIME_LIBRARY       Boost::date_time)
  set(Boost_FILESYSTEM_LIBRARY     Boost::filesystem)
  set(Boost_IOSTREAMS_LIBRARY      Boost::iostreams)
  set(Boost_PYTHON_LIBRARY         Boost::python39)
  set(Boost_PROGRAMOPTIONS_LIBRARY Boost::program_options)
  set(Boost_REGEX_LIBRARY          Boost::regex)
  set(Boost_SYSTEM_LIBRARY         Boost::system)
  set(Boost_THREAD_LIBRARY         Boost::thread)
endif()

list(APPEND BOOST_LIBRARIES
  ${Boost_ATOMIC_LIBRARY}
  ${Boost_DATETIME_LIBRARY}
  ${Boost_CHRONO_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_IOSTREAMS_LIBRARY}
  ${Boost_PYTHON_LIBRARY}
  ${Boost_NUMPY_LIBRARY}
  ${Boost_PROGRAMOPTIONS_LIBRARY}
  ${Boost_REGEX_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
)

# Disable superfluous Boost Warnings
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

#-------------------------------------------------------------------------------------------------------------------------------------------
# Find Jinja2

find_package(Jinja2)
if(NOT JINJA2_FOUND)
  execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install jinja2)
  find_package(Jinja2 REQUIRED)
endif()

#-------------------------------------------------------------------------------------------------------------------------------------------
# Find Doxygen

if(WABI_BUILD_DOCUMENTATION)

  find_program(DOXYGEN_EXECUTABLE
    HINTS "C:/Program Files/doxygen"
    NAMES doxygen
  )
  if(NOT EXISTS ${DOXYGEN_EXECUTABLE})
    set(WABI_BUILD_DOCUMENTATION OFF)
  endif()

  find_program(DOT_EXECUTABLE
    HINTS "C:/Program Files/Graphiz/bin"
    NAMES dot
  )
  if(NOT EXISTS ${DOT_EXECUTABLE})
    set(WABI_BUILD_DOCUMENTATION OFF)
  endif()

endif()

#-------------------------------------------------------------------------------------------------------------------------------------------
# Find Threading Building Blocks

if(WIN32)
  set(TBB_ROOT "${LIBDIR}/tbb")
  find_package(TBB REQUIRED COMPONENTS tbb)
  add_definitions(${TBB_DEFINITIONS})
  if(KRAKEN_RELEASE_MODE)
    list(APPEND TBB_LIBRARIES
      "${LIBDIR}/tbb/lib/tbb.lib"
      "${LIBDIR}/tbb/lib/tbbmalloc.lib"
      "${LIBDIR}/tbb/lib/tbbmalloc_proxy.lib"
    )
  else()
    list(APPEND TBB_LIBRARIES
      "${LIBDIR}/tbb/lib/tbb_debug.lib"
      "${LIBDIR}/tbb/lib/tbbmalloc_debug.lib"
      "${LIBDIR}/tbb/lib/tbbmalloc_proxy_debug.lib"
    )    
  endif()
elseif(UNIX)
  # Enable TBBs Ability to wait for the completion
  # of worker threads.
  find_package(TBB REQUIRED COMPONENTS tbb)
  add_definitions(-DTBB_PREVIEW_WAITING_FOR_WORKERS=1)
  add_definitions(-DTBB_PREVIEW_ISOLATED_TASK_GROUP=1)
endif()

# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx math xxxxx
if(WIN32)
  # Math functions are linked automatically by including math.h on Windows.
  set(M_LIB "")
else()
  find_library(M_LIB m)
endif()

if(WIN32)
  list(APPEND WABI_MALLOC_LIBRARY
    ${LIBDIR}/mimalloc/lib/mimalloc-2.0/mimalloc.lib
    ${LIBDIR}/mimalloc/lib/mimalloc-2.0/mimalloc-static.lib)
else()
  find_package(Jemalloc REQUIRED)
  set(WABI_MALLOC_LIBRARY ${JEMALLOC_LIBRARY})
endif()

if(WABI_VALIDATE_GENERATED_CODE)
  if(WIN32)
    set(BISON_FOUND ON)
    set(BISON_EXECUTABLE "${LIBDIR}/flex/win_bison.exe")
    set(FLEX_FOUND ON)
    set(FLEX_EXECUTABLE "${LIBDIR}/flex/win_flex.exe")
  else()
    find_package(BISON REQUIRED)
    find_package(FLEX REQUIRED)
  endif()
endif()

# Imaging Components Package Requirements
# ----------------------------------------------

if(WABI_BUILD_IMAGING)
  # --OpenImageIO
  if(WITH_OPENIMAGEIO)
    if(UNIX)
      find_package(OpenEXR REQUIRED)
    endif()
    find_package(OpenImageIO REQUIRED)
    add_definitions(-DWITH_OPENIMAGEIO)
  endif()
  # --OpenColorIO
  if(WITH_OPENCOLORIO)
    find_package(OpenColorIO REQUIRED)
    add_definitions(-DWITH_OPENCOLORIO)
  endif()
  # --Opensubdiv
  set(OPENSUBDIV_USE_GPU ${WITH_GPU_SUPPORT})
  find_package(OpenSubdiv 3 REQUIRED)
  # --Ptex
  if(WITH_PTEX)
    find_package(PTex REQUIRED)
    add_definitions(-DWITH_PTEX)
  endif()
  # --OpenVDB
  if(WITH_OPENVDB)
    if(NOT WIN32)
      find_package(OpenEXR REQUIRED)
    endif()
    find_package(OpenVDB REQUIRED)
    add_definitions(-DWITH_OPENVDB)
  endif()
  # --Embree
  if(WITH_EMBREE)
    find_package(Embree REQUIRED)
    add_definitions(-DWITH_EMBREE)
  endif()
endif()

if(WITH_PIXAR_USDVIEW)
  # --PySide
  if(UNIX)
    find_package(PySide)
  elseif(WIN32)
    set(PYSIDE_FOUND ON)
    set(PYSIDE_BIN_DIR ${LIBDIR}/python/${python_version_nodot}/lib/site-packages/PySide2)
    set(PYSIDEUICBINARY ${LIBDIR}/python/${python_version_nodot}/lib/site-packages/PySide2/uic.exe)
  endif()
  # --PyOpenGL
  if(UNIX)
    find_package(PyOpenGL)
  elseif(WIN32)
    set(PYOPENGL_AVAILABLE ON)
  endif()
endif()

if(WIN32)
  set(JINJA2_FOUND ON)
endif()

# Third Party Plugin Package Requirements
# ----------------------------------------------
if(WITH_RENDERMAN)
  if(NOT EXISTS $ENV{RMANTREE})
    # Attempt to find RenderMan installation.
    if(UNIX)
      set(RENDERMAN_LOCATION "/opt/pixar/RenderManProServer-24.0")
    elseif(WIN32)
      set(RENDERMAN_LOCATION "C:/Program Files/Pixar/RenderManProServer-24.0")
    endif()
  endif()
  find_package(Renderman REQUIRED)
endif()

if(WITH_ARNOLD)
  add_definitions(-DWITH_ARNOLD)
  if(UNIX)
    set(ARNOLD_HOME ${LIBDIR})
  elseif(WIN32)
    set(ARNOLD_HOME ${LIBDIR}/arnold)
  endif()
  find_package(Arnold REQUIRED)
endif()

if(KRAKEN_RELEASE_MODE)
  if(WITH_CYCLES)
    add_definitions(-DWITH_CYCLES)
    if(UNIX)
      set(CYCLES_HOME ${LIBDIR})
    elseif(WIN32)
      if(KRAKEN_RELEASE_MODE)
        set(CYCLES_HOME ${LIBDIR}/Cycles)
      else()
        set(CYCLES_HOME ${LIBDIR}/Cycles/debug)
      endif()
    endif()
    find_package(Cycles REQUIRED)
    find_package(OpenImageDenoise REQUIRED)
    if(WIN32)
      if(KRAKEN_RELEASE_MODE)
        set(CYCLES_INCLUDE_DIRS ${CYCLES_INCLUDE_DIRS} ${LIBDIR}/Cycles/include)
      else()
        set(CYCLES_INCLUDE_DIRS ${CYCLES_INCLUDE_DIRS} ${LIBDIR}/Cycles/debug/include)
      endif()
    endif()
  endif()
else()
  # Temporarily disable Cycles for Debug Builds.
  set(WITH_CYCLES OFF)
endif()

if(WITH_PRORENDER)
  add_definitions(-DWITH_PRORENDER)
  if(UNIX)
    set(PRORENDER_HOME ${LIBDIR})
  elseif(WIN32)
    set(PRORENDER_HOME ${LIBDIR}/prorender)
  endif()
  find_package(ProRender REQUIRED)
endif()

if(WITH_ALEMBIC)
  find_package(HDF5 REQUIRED)
  find_package(Alembic REQUIRED)

  if(WITH_ALEMBIC_HDF5)
    find_package(HDF5 REQUIRED COMPONENTS HL REQUIRED)
  endif()
  add_definitions(-D_USE_MATH_DEFINES=1)
  add_definitions(-DWITH_ALEMBIC)
endif()

if(WITH_DRACO)
  if(WIN32)
    set(DRACO_FOUND ON)
    set(DRACO_LIBRARY ${LIBDIR}/draco/lib/draco.lib)
    set(DRACO_INCLUDES ${LIBDIR}/draco/include)
  else()
    find_package(Draco REQUIRED)
  endif()
  add_definitions(-DWITH_DRACO)
endif()

if(WITH_MATERIALX)
  find_package(MaterialX REQUIRED)
  add_definitions(-DWITH_MATERIALX)
endif()

if(WITH_OSL)
  add_definitions(-DWITH_OSL)
  find_package(OSL REQUIRED)
endif()

if(WIN32)
  if(KRAKEN_RELEASE_MODE)
    # Fix Debug vs Release Libraries.
    # Leave Release Libraies untouched
    # as they are properly found via
    # find_package(XXX) above. But
    # Override the libraries to link
    # with the debug libs if build not
    # KRAKEN_RELEASE_MODE.
  else()
    set(OPENSUBDIV_LIBRARIES 
      ${LIBDIR}/opensubdiv/lib/debug/osdCPU.lib
      ${LIBDIR}/opensubdiv/lib/debug/osdGPU.lib
    )
    set(MATERIALX_LIBRARIES
      ${LIBDIR}/MaterialX/lib/debug/MaterialXCore.lib
      ${LIBDIR}/MaterialX/lib/debug/MaterialXFormat.lib
      ${LIBDIR}/MaterialX/lib/debug/MaterialXGenGlsl.lib
      ${LIBDIR}/MaterialX/lib/debug/MaterialXGenMdl.lib
      ${LIBDIR}/MaterialX/lib/debug/MaterialXGenOsl.lib
      ${LIBDIR}/MaterialX/lib/debug/MaterialXGenShader.lib
      ${LIBDIR}/MaterialX/lib/debug/MaterialXRender.lib
      ${LIBDIR}/MaterialX/lib/debug/MaterialXRenderGlsl.lib
      ${LIBDIR}/MaterialX/lib/debug/MaterialXRenderHw.lib
      ${LIBDIR}/MaterialX/lib/debug/MaterialXRenderOsl.lib
    )
    set(OPENVDB_LIBRARY
      ${LIBDIR}/openvdb/lib/openvdb_d.lib
    )
  endif()
endif()

# ----------------------------------------------

set(BUILD_SHARED_LIBS "${build_shared_libs}")
