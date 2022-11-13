if(CMAKE_COMPILER_IS_GNUCC)
  # add here so we fail early.
  string(APPEND CMAKE_CXX_FLAGS " -Werror=implicit-function-declaration")
endif()

# Needed for `mallocn.c`.
if(HAVE_MALLOC_STATS_H)
  add_definitions(-DHAVE_MALLOC_STATS_H)
endif()

set(DEFSRC
  LUXO_context.cpp
  LUXO_info.cpp
  LUXO_id.cpp
  LUXO_luxo.cpp
  LUXO_main.cpp
  LUXO_wm.cpp
)

string(REGEX REPLACE "LUXO_([a-zA-Z0-9_-]*).cpp" "${CMAKE_CURRENT_BINARY_DIR}/LUXO_\\1_gen.cpp" GENSRC "${DEFSRC}")
set(GENFILES
  "${CMAKE_CURRENT_BINARY_DIR}/LUXO_prototypes_gen.h"
  # "${CMAKE_CURRENT_BINARY_DIR}/../LUXO_prototypes.h"
)
set_source_files_properties(${GENFILES} PROPERTIES GENERATED TRUE)

# --------------------------
# CXXFLAGS for Generated Files
#
# less strict flags for generated source
set(GENSRC_CXXFLAGS)
if(CMAKE_COMPILER_IS_GNUCC OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
  set(GENSRC_CXXFLAGS "-Wno-missing-prototypes")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  string(APPEND GENSRC_CXXFLAGS " -Wno-missing-variable-declarations")
endif()

if(GENSRC_CXXFLAGS)
  set_source_files_properties(${GENFILES} PROPERTIES COMPILE_FLAGS "${GENSRC_CXXFLAGS}")
endif()
unset(GENSRC_CXXFLAGS)

# NOTE: Disable clang-tidy because generated files are stored outside of the source,
# so the clang-tidy can not find our .clang-tidy and fall-backs to own set of rules
# which are too noisy for Blender.
#
# In the future clang-tidy would either need to be inlined checks and passed via the
# command line (instead of using .clang-tidy file). Or, maybe, there is a way to
# pass configuration file to the clang-tidy command.
unset(CMAKE_C_CLANG_TIDY)
unset(CMAKE_CXX_CLANG_TIDY)

set(SRC_LUXO_INC
  ../LUXO_define.h
  ../LUXO_enum_items.h
  ../LUXO_enum_types.h
  ../LUXO_main.h
  ../LUXO_runtime.h
  ../LUXO_types.h
)

set(SRC
  LUXO_define.cpp
  ${DEFSRC}
  ${APISRC}
  # ../../../../intern/guardedalloc/intern/leak_detector.cc
  # ../../../../intern/guardedalloc/intern/mallocn.c
  # ../../../../intern/guardedalloc/intern/mallocn_guarded_impl.c
  # ../../../../intern/guardedalloc/intern/mallocn_lockfree_impl.c

  # Needed for defaults.
  # ../../../../release/datafiles/userdef/userdef_default.c
  # ../../../../release/datafiles/userdef/userdef_default_theme.c
)

set(INC
  .
  ..
  ../../krakfont
  ../../krakernel
  ../../kraklib
  ../../gpu
  ../../imbuf
  ../../universe
  ../../wm
  ../../editors/include
  ../../../../. # Pixar USD
  ../../../../source
  ../../../../intern/atomic
  ../../../../intern/clog
  ../../../../intern/guardedalloc
  ../../../../extern/wcwidth

  ${CMAKE_BINARY_DIR}/source/kraken/universe/intern

  ${CMAKE_CURRENT_BINARY_DIR}/../../universe/intern
  ${CMAKE_CURRENT_BINARY_DIR}/../../luxo/
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
  ${PTHREADS_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIR}
  ${EIGEN3_INCLUDE_DIRS}
)

if(WITH_PYTHON)
  add_definitions(-DWITH_PYTHON)
  list(APPEND INC
    ../../python
  )
endif()

# Build luxo executable
add_cc_flags_custom_test(luxo)

list(APPEND COMBINED_INC
  ${SRC_LUXO_INC}
  ${SRC_USD_INC}
)

kraken_add_lib(luxo "${SRC}" "${COMBINED_INC}" "${INC_SYS}" "")
setup_platform_linker_flags(luxo)
kraken_target_include_dirs(luxo ${INC})
kraken_target_include_dirs_sys(luxo ${INC_SYS})

target_link_libraries(luxo PUBLIC kraken_universe)
# target_link_libraries(luxo wabi_usd_krakenlib)

if(WIN32 AND NOT UNIX)
  target_link_libraries(luxo ${PTHREADS_LIBRARIES})
endif()

# Output LUXO_*_gen.cpp
# note (linux only): with crashes try add this after COMMAND: valgrind --leak-check=full --track-origins=yes
add_custom_command(
  OUTPUT ${GENFILES}
  # COMMAND "$<TARGET_FILE:luxo>" ${CMAKE_CURRENT_BINARY_DIR}/  ${CMAKE_CURRENT_BINARY_DIR}/../
  COMMAND pwsh -Command "New-Item ${GENFILES} -Force"
  DEPENDS luxo
)

# Build kraken_luxo
set(SRC
  LUXO_access.cpp
  ${GENFILES}

  ${SRC_LUXO_INC}
  ../LUXO_access.h
  LUXO_internal.h
)

set(LIB
  kraken_universe
  kraken_editor_space_api

  kraken_editor_object
  kraken_editor_interface
  kraken_editor_spacefile
  kraken_editor_screen
  kraken_editor_space_view3d
  kraken_editor_space_userpref
  luxo
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${PTHREADS_LIBRARY}

  # maelstrom
  # maelstrom_static
  # kraken_lib
  # kraken_kernel
  # kraken_universe
)

kraken_add_lib(kraken_luxo "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

add_dependencies(kraken_kernel kraken_universe)