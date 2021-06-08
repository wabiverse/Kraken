cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0005 NEW)
cmake_policy(SET CMP0010 NEW)
cmake_policy(SET CMP0014 NEW)

project(COVAHMultiTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../..//build_files/cmake/Modules")

# Fake the current source dir for includes.
set(_CMAKE_CURRENT_SOURCE_DIR_INIT "${CMAKE_CURRENT_SOURCE_DIR}")

# -----------------------------------------------------------------------------
# Macros

# include(../../../build_files/cmake/macros.cmake)

# stub macro, does nothing
macro(covah_add_lib
  name
  sources
  includes
  includes_sys
  library_deps # ignored.
  )

endmacro()

# suffix relative paths so we can use external cmake files
macro(suffix_relpaths
  new_files files prefix)

  set(${new_files})
  foreach(_file ${files})
    if(IS_ABSOLUTE "${_file}")
      list(APPEND ${new_files} ${_file})
    else()
      set(_file_to "${prefix}${_file}")
      list(APPEND ${new_files} ${_file_to})
      get_source_file_property(_is_generated ${_file} GENERATED)
      set_source_files_properties(${_file_to} PROPERTIES GENERATED ${_is_generated})
      unset(_file_to)
      unset(_is_generated)
    endif()
  endforeach()
  unset(_file)
endmacro()

# -----------------------------------------------------------------------------
# Defines

# set the endian define
if(MSVC)
  # for some reason this fails on msvc
  add_definitions(-D__LITTLE_ENDIAN__)
else()
  include(TestBigEndian)
  test_big_endian(_SYSTEM_BIG_ENDIAN)
  if(_SYSTEM_BIG_ENDIAN)
    add_definitions(-D__BIG_ENDIAN__)
  else()
    add_definitions(-D__LITTLE_ENDIAN__)
  endif()
  unset(_SYSTEM_BIG_ENDIAN)
endif()

# -----------------------------------------------------------------------------
# Libraries

find_package(OpenGL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/../)
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${TBB_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

if(CMAKE_COMPILER_IS_GNUCC)
  set(PLATFORM_CFLAGS "-funsigned-char")
endif()

string(APPEND CMAKE_C_FLAGS " ${PLATFORM_CFLAGS}")
string(APPEND CMAKE_CXX_FLAGS " ${PLATFORM_CFLAGS}")

# -----------------------------------------------------------------------------
# Executables

# Plugs
add_executable(plugs_test ${CMAKE_SOURCE_DIR}/source/covah/test/plugs/plugs_test.cpp)

target_link_libraries(plugs_test
  ${OPENGL_gl_LIBRARY}
  ${ZLIB_LIBRARY}
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${CMAKE_DL_LIBS}
  ${PLATFORM_LINKLIBS}
)
