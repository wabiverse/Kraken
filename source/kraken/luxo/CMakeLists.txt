if(CMAKE_COMPILER_IS_GNUCC)
  # add here so we fail early.
  string(APPEND CMAKE_C_FLAGS " -Werror=implicit-function-declaration")
endif()

# Needed for `mallocn.c`.
# if(HAVE_MALLOC_STATS_H)
#   add_definitions(-DHAVE_MALLOC_STATS_H)
# endif()

set(DEFSRC
  intern/LUXO_info.cpp
  intern/LUXO_main.cpp
  intern/LUXO_wm.cpp
)

string(REGEX REPLACE "rna_([a-zA-Z0-9_-]*).c" "${CMAKE_CURRENT_BINARY_DIR}/LUXO_\\1_gen.c" GENSRC "${DEFSRC}")
list(APPEND GENSRC
  "${CMAKE_CURRENT_BINARY_DIR}/LUXO_prototypes_gen.h"
  "${CMAKE_CURRENT_BINARY_DIR}/../PRIM_prototypes.h"
)
set_source_files_properties(${GENSRC} PROPERTIES GENERATED TRUE)

# --------------------------
# CFLAGS for Generated Files
#
# less strict flags for generated source
set(GENSRC_CFLAGS)
if(CMAKE_COMPILER_IS_GNUCC OR (CMAKE_C_COMPILER_ID MATCHES "Clang"))
  set(GENSRC_CFLAGS "-Wno-missing-prototypes")
endif()
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  string(APPEND GENSRC_CFLAGS " -Wno-missing-variable-declarations")
endif()

if(GENSRC_CFLAGS)
  set_source_files_properties(${GENSRC} PROPERTIES COMPILE_FLAGS "${GENSRC_CFLAGS}")
endif()
unset(GENSRC_CFLAGS)

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
  LUXO_define.h
  LUXO_main.h
  LUXO_runtime.h
)

set(SRC
  intern/LUXO_define.cpp
  ${DEFSRC}
  ${APISRC}
  # ../../../../intern/clog/clog.c
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
  ../../../. # Pixar USD
  ../../../source
  ../editors/include
  ../universe
  ../krakernel
  ../kraklib
  ../wm

  ${CMAKE_BINARY_DIR}/source/kraken/luxo/intern

  ${CMAKE_CURRENT_BINARY_DIR}/../../universe/intern
  ${CMAKE_CURRENT_BINARY_DIR}/../../luxo/
)

set(INC_SYS

)

# Build luxo executable
add_cc_flags_custom_test(luxo)

add_executable(luxo ${SRC} ${SRC_LUXO_INC} ${SRC_USD_INC})
setup_platform_linker_flags(luxo)
kraken_target_include_dirs(luxo ${INC})
kraken_target_include_dirs_sys(luxo ${INC_SYS})

target_link_libraries(luxo kraken_universe)
# target_link_libraries(luxo wabi_usd_krakenlib)

if(WIN32 AND NOT UNIX)
  target_link_libraries(luxo ${PTHREADS_LIBRARIES})
endif()

# Output rna_*_gen.c
# note (linux only): with crashes try add this after COMMAND: valgrind --leak-check=full --track-origins=yes
add_custom_command(
  OUTPUT ${GENSRC}
  COMMAND "$<TARGET_FILE:luxo>" ${CMAKE_CURRENT_BINARY_DIR}/  ${CMAKE_CURRENT_BINARY_DIR}/../
  DEPENDS luxo
)

# Build kraken_luxo
set(SRC
  intern/LUXO_access.cpp
  ${GENSRC}

  ${SRC_LUXO_INC}
  LUXO_access.h
  intern/LUXO_internal.h
)

set(LIB
  kraken_universe
  kraken_editor_interface
  kraken_editor_spacefile
  kraken_editor_screen
  kraken_editor_spaceview3d

  # maelstrom
  # maelstrom_static
  # kraken_lib
  # kraken_kernel
  # kraken_universe
)

kraken_add_lib(kraken_luxo "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

add_dependencies(kraken_kernel kraken_universe)