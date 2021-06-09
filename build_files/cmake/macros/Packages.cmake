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
  set(LIBPATH ${CMAKE_SOURCE_DIR}/../lib/win64_vc15)
  set(LIB_OBJ_EXT "lib")
elseif(UNIX)
  set(LIBPATH ${CMAKE_SOURCE_DIR}/../lib/linux_centos7_x86_64)
  set(LIB_OBJ_EXT "so")
endif()

# ! Important
# Convert the relative path to an absolute path.
string(REPLACE "covah/../" "" LIBDIR "${LIBPATH}")

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
  set(OPENEXR_LOCATION    ${LIBDIR}/openexr)
  set(OCIO_LOCATION       ${LIBDIR}/opencolorio)
  set(OIIO_LOCATION       ${LIBDIR}/OpenImageIO)
  set(ZLIB_INCLUDE_DIR    ${LIBDIR}/zlib/include)
  set(ZLIB_LIBRARY        ${LIBDIR}/zlib/lib/libz_st.lib)
  set(OPENSUBDIV_ROOT_DIR ${LIBDIR}/opensubdiv)
  set(PTEX_LOCATION       ${LIBDIR}/ptex)
  set(OPENVDB_LOCATION    ${LIBDIR}/openvdb)
  set(ALEMBIC_DIR         ${LIBDIR}/alembic)
  set(OSL_LOCATION        ${LIBDIR}/osl)
  set(EMBREE_LOCATION     ${LIBDIR}/embree)
  set(MATERIALX_ROOT      ${LIBDIR}/MaterialX)
  set(MATERIALX_DATA_ROOT ${LIBDIR}/MaterialX/libraries)
  set(TBB_ROOT_DIR        ${LIBDIR}/tbb)

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

#-------------------------------------------------------------------------------------------------------------------------------------------
# Find Python

if(WIN32)
  find_package(PythonInterp 3.9 REQUIRED)
  set(python_version_nodot "${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}")
  set(PYTHON_INCLUDE_DIR ${LIBDIR}/python/${python_version_nodot}/include)
  set(PYTHON_LIBRARIES ${LIBDIR}/python/${python_version_nodot}/libs/python${python_version_nodot}.lib)
else()
  find_package(Python 3.9 COMPONENTS Interpreter Development REQUIRED)
  set(PYTHON_INCLUDE_DIR ${Python_INCLUDE_DIRS})
  set(PYTHON_LIBPATH ${Python_LIBRARY_DIRS})
  set(PYTHON_LIBRARIES ${Python_LIBRARIES})
  set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
endif()

#-------------------------------------------------------------------------------------------------------------------------------------------
# Find Freetype

find_package(Freetype REQUIRED)

#-------------------------------------------------------------------------------------------------------------------------------------------
# Configure GFX APIs

# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx OpenGL xxxxx
if(WITH_OPENGL)
  add_definitions(-DWITH_OPENGL)
  find_package(OpenGL REQUIRED)
  covah_include_dirs_sys("${OPENGL_INCLUDE_DIR}")
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
    glslang
    OGLCompiler
    OSDependent
    MachineIndependent
    GenericCodeGen
    SPIRV SPIRV-Tools
    SPIRV-Tools-opt
    SPIRV-Tools-shared
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
    # No extra libs required
  endif()

  add_definitions(-DWITH_VULKAN)

  set(VULKAN_INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS})
  list(APPEND VULKAN_LIBRARIES
    ${Vulkan_LIBRARIES}
    ${VULKAN_LIBS}
  )

  covah_include_dirs_sys("${VULKAN_INCLUDE_DIRS}")

else()
  message(FATAL_ERROR "Vulkan Installation not valid")
endif()

if(UNIX)
  find_package(X11)
endif()

if(UNIX)
  set(SDL2_INCLUDE_DIR /usr/include/SDL2)
  list(APPEND SDL2_LIBRARIES
    /usr/lib64/libSDL2.so
    /usr/lib64/libSDL2_image.so
    /usr/lib64/libSDL2_ttf.so)
endif()

#-------------------------------------------------------------------------------------------------------------------------------------------
# REST - For Licensing

if(WIN32)
  set(REST_INCLUDE_DIR ${LIBDIR}/rest/include)
  set(REST_LIBRARIES
    ${LIBDIR}/rest/lib/cpprest_2_10.lib
  )
endif()

#-------------------------------------------------------------------------------------------------------------------------------------------
# Find BOOST

if(WIN32)
  set(BOOST_VERSION_SCORE "1_76")
  set(BOOST_LIBRARY_SUFFIX "vc142-mt-x64-1_76")
  # set(Boost_USE_STATIC_RUNTIME ON) # prefix lib
  # set(Boost_USE_MULTITHREADED ON) # suffix -mt
  # set(Boost_USE_STATIC_LIBS ON) # suffix -s
  set(BOOST_ROOT "${LIBDIR}/boost")
  find_package(Boost REQUIRED)
  set(boost_version_string "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")

  set(Boost_INCLUDE_DIRS           ${BOOST_ROOT}/include/boost-${BOOST_VERSION_SCORE})
  set(Boost_ATOMIC_LIBRARY         ${BOOST_ROOT}/lib/boost_atomic-${BOOST_LIBRARY_SUFFIX}.${LIB_OBJ_EXT})
  set(Boost_CHRONO_LIBRARY         ${BOOST_ROOT}/lib/boost_chrono-${BOOST_LIBRARY_SUFFIX}.${LIB_OBJ_EXT})
  set(Boost_DATETIME_LIBRARY       ${BOOST_ROOT}/lib/boost_date_time-${BOOST_LIBRARY_SUFFIX}.${LIB_OBJ_EXT})
  set(Boost_FILESYSTEM_LIBRARY     ${BOOST_ROOT}/lib/boost_filesystem-${BOOST_LIBRARY_SUFFIX}.${LIB_OBJ_EXT})
  set(Boost_IOSTREAMS_LIBRARY      ${BOOST_ROOT}/lib/boost_iostreams-${BOOST_LIBRARY_SUFFIX}.${LIB_OBJ_EXT})
  set(Boost_NUMPY_LIBRARY          ${BOOST_ROOT}/lib/boost_numpy${python_version_nodot}-${BOOST_LIBRARY_SUFFIX}.${LIB_OBJ_EXT})
  set(Boost_PYTHON_LIBRARY         ${BOOST_ROOT}/lib/boost_python${python_version_nodot}-${BOOST_LIBRARY_SUFFIX}.${LIB_OBJ_EXT})
  set(Boost_PROGRAMOPTIONS_LIBRARY ${BOOST_ROOT}/lib/boost_program_options-${BOOST_LIBRARY_SUFFIX}.${LIB_OBJ_EXT})
  set(Boost_REGEX_LIBRARY          ${BOOST_ROOT}/lib/boost_regex-${BOOST_LIBRARY_SUFFIX}.${LIB_OBJ_EXT})
  set(Boost_SYSTEM_LIBRARY         ${BOOST_ROOT}/lib/boost_system-${BOOST_LIBRARY_SUFFIX}.${LIB_OBJ_EXT})
  set(Boost_THREAD_LIBRARY         ${BOOST_ROOT}/lib/boost_thread-${BOOST_LIBRARY_SUFFIX}.${LIB_OBJ_EXT})

elseif(UNIX)
  set(BOOST_ROOT "${LIBDIR}")
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
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_IOSTREAMS_LIBRARY}
  ${Boost_PYTHON_LIBRARY}
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
  list(APPEND TBB_LIBRARIES
    "${LIBDIR}/tbb/lib/tbb.lib"
    "${LIBDIR}/tbb/lib/tbb12.lib"
    "${LIBDIR}/tbb/lib/tbbbind_2_0.lib"
    "${LIBDIR}/tbb/lib/tbbmalloc.lib"
    "${LIBDIR}/tbb/lib/tbbmalloc_proxy.lib"
  )
elseif(UNIX)
  find_package(TBB REQUIRED COMPONENTS tbb)
endif()

# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx math xxxxx
if(WIN32)
  # Math functions are linked automatically by including math.h on Windows.
  set(M_LIB "")
else()
  find_library(M_LIB m)
endif()

if(WIN32)
  set(WABI_MALLOC_LIBRARY ${LIBDIR}/jemalloc/jemalloc-vc142-static.lib)
else()
  find_package(Jemalloc REQUIRED)
  set(WABI_MALLOC_LIBRARY ${JEMALLOC_LIBRARY})
endif()

if(NOT EXISTS ${WABI_MALLOC_LIBRARY})
  message(STATUS "Using default system allocator because WABI_MALLOC_LIBRARY is unspecified")
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

if(WABI_BUILD_USDVIEW)
  # --PySide

  find_package(PySide)
  if(NOT PYSIDE_FOUND)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install PySide2)
    find_package(PySide REQUIRED)
  endif()
  # --PyOpenGL
  find_package(PyOpenGL)
  if(NOT PYOPENGL_FOUND)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install PyOpenGL)
    find_package(PyOpenGL REQUIRED)
  endif()
endif()

# Third Party Plugin Package Requirements
# ----------------------------------------------
if(WITH_RENDERMAN)
  if(NOT EXISTS $ENV{RMANTREE})
    # Attempt to find RenderMan installation.
    if(UNIX)
      set(RENDERMAN_LOCATION "/opt/pixar/RenderManProServer-23.5")
    elseif(WIN32)
      set(RENDERMAN_LOCATION "C:/Program Files/RenderManProServer-23.5")
    endif()
  endif()
  find_package(Renderman REQUIRED)
endif()

if(WITH_ARNOLD)
  add_definitions(-DWITH_ARNOLD)
  set(ARNOLD_HOME ${LIBDIR})
  find_package(Arnold REQUIRED)
endif()

if(WITH_CYCLES)
  add_definitions(-DWITH_CYCLES)
  set(CYCLES_HOME ${LIBDIR})
  find_package(Cycles REQUIRED)
  find_package(OpenImageDenoise REQUIRED)
endif()

if(WITH_PRORENDER)
  add_definitions(-DWITH_PRORENDER)
  set(PRORENDER_HOME ${LIBDIR})
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

# ----------------------------------------------

set(BUILD_SHARED_LIBS "${build_shared_libs}")
