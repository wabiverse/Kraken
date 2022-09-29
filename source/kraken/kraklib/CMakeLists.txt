set(INC
  .
  ../../../. # Pixar USD
  ../../../source
  ../eigen
  ../krakernel
  ../universe
  ../luxo
  ../editors/include
)

set(SRC
  intern/KLI_assert.c
  intern/KLI_filelist.cpp
  intern/KLI_fileops.cpp
  intern/KLI_icons.cpp
  intern/KLI_listbase.cpp
  intern/KLI_math_base_inline.c
  intern/KLI_math_bits_inline.c
  intern/KLI_math_geom_inline.c
  intern/KLI_math_geom.c
  intern/KLI_math_matrix.c
  intern/KLI_math_rotation.c
  intern/KLI_math_solvers.c
  intern/KLI_math_vector_inline.c
  intern/KLI_math_vector.c
  intern/KLI_numaapi.cpp
  intern/KLI_path_utils.cpp
  intern/KLI_rect.cpp
  intern/KLI_string_utf8.c
  intern/KLI_storage.cpp
  intern/KLI_system.c
  intern/KLI_string.c
  intern/KLI_time.cpp
  intern/KLI_threads.cpp
  intern/KLI_winstuff.cpp

  KLI_api.h
  KLI_array.h
  KLI_assert.h
  KLI_compiler_attrs.h
  KLI_compiler_compat.h
  KLI_compiler_typecheck.h
  KLI_dynstr.h
  KLI_easing.h
  KLI_fileops.h
  KLI_index_range.hh
  KLI_icons.h
  KLI_kraklib.h
  KLI_link_utils.h
  KLI_listbase.h
  KLI_memory_utils.hh
  KLI_math_base.h
  KLI_math_bits.h
  KLI_math_geom.h
  KLI_math_inline.h
  KLI_math_matrix.h
  KLI_math_rotation.h
  KLI_math_solvers.h
  KLI_math_vector.h
  KLI_math.h
  KLI_numaapi.h
  KLI_path_utils.h
  KLI_rect.h
  KLI_simd.h
  KLI_span.hh
  KLI_strict_flags.h
  KLI_string_ref.hh
  KLI_string_utils.h
  KLI_string_utf8.h
  KLI_string.h
  KLI_sys_types.h
  KLI_system.h
  KLI_threads.h
  KLI_time.h
  KLI_utildefines.h
  KLI_utildefines_variadic.h
  KLI_winstuff.h
  KLI_vector.hh
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${PTHREADS_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIR}
  ${EIGEN3_INCLUDE_DIRS}
)

list(APPEND LIB
  kraken_intern_eigen
  kraken_intern_guardedalloc
  maelstrom
  maelstrom_static
  kraken_kernel
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${PTHREADS_LIBRARY}
  ${ZLIB_LIBRARIES}
  ${ZSTD_LIBRARIES}
)

if(WIN32)
  list(APPEND INC
    ../../../intern/utfconv
  )

  list(APPEND LIB
    kraken_intern_utfconv
    version.lib
  )

  list(APPEND SRC
    intern/KLI_system_win32.cpp
  )
endif()

add_definitions(-DKRAKEN_LIB_EXPORTS)

if(APPLE)
  list(APPEND SRC
    intern/storage_apple.mm
  )
endif()

kraken_add_lib(kraken_lib "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

add_dependencies(kraken_lib kraken_intern_eigen)