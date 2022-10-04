set(INC
  .
  ../../../. # Pixar USD
  ../../../source
  ../eigen
  ../krakernel
  ../universe
  ../luxo
  ../editors/include
  ../../../extern/wcwidth
  ../../../intern/atomic
  ../../../intern/clog
  ../../../intern/guardedalloc
)

set(SRC
  intern/KLI_assert.c
  intern/KLI_dynstr.c
  intern/KLI_filelist.cpp
  intern/KLI_listbase.cpp
  intern/KLI_math_base_inline.c
  intern/KLI_math_base_safe_inline.c
  intern/KLI_math_bits_inline.c
  intern/KLI_math_color_inline.c
  intern/KLI_math_geom.c
  intern/KLI_math_geom_inline.c
  intern/KLI_math_matrix.c
  intern/KLI_math_rotation.c
  intern/KLI_math_solvers.c
  intern/KLI_math_vector.c
  intern/KLI_math_vector_inline.c
  intern/KLI_memarena.c
  intern/KLI_mempool.c
  intern/KLI_numaapi.cpp
  intern/path_util.c
  intern/KLI_rect.cpp
  intern/KLI_rhash.c
  intern/KLI_storage.cpp
  intern/KLI_string.c
  intern/KLI_string_utf8.c
  intern/KLI_string_utils.c
  intern/KLI_system.c
  intern/KLI_threads.cpp
  intern/KLI_time.cpp
  intern/KLI_winstuff.cpp
  intern/fileops.c
  intern/fileops.cc
  intern/fnmatch.c
  intern/gsqueue.c
  intern/hash_mm2a.c
  intern/hash_mm3.c
  intern/lazy_threading.cc
  intern/listbase.c
  intern/task_scheduler.cc

  intern/KLI_mempool_private.h
  intern/list_sort_impl.h

  KLI_asan.h
  KLI_allocator.hh
  KLI_api.h
  KLI_array.h
  KLI_array.hh
  KLI_assert.h
  KLI_compiler_attrs.h
  KLI_compiler_compat.h
  KLI_compiler_typecheck.h
  KLI_dynstr.h
  KLI_easing.h
  KLI_fileops.h
  KLI_fileops.hh
  KLI_fileops_types.h
  KLI_float4x4.hh
  KLI_fnmatch.h
  KLI_function_ref.hh
  KLI_gsqueue.h
  KLI_hash.h
  KLI_hash.hh
  KLI_hash_mm2a.h
  KLI_hash_mm3.h
  KLI_hash_tables.hh
  KLI_index_range.hh
  KLI_kraklib.h
  KLI_lazy_threading.hh
  KLI_link_utils.h
  KLI_listbase.h
  KLI_listbase_wrapper.hh
  KLI_map.hh
  KLI_map_slots.hh
  KLI_math.h
  KLI_math_base.h
  KLI_math_base.hh
  KLI_math_base_safe.h
  KLI_math_bits.h
  KLI_math_color.h
  KLI_math_geom.h
  KLI_math_inline.h
  KLI_math_matrix.h
  KLI_math_rotation.h
  KLI_math_solvers.h
  KLI_math_vec_types.hh
  KLI_math_vector.h
  KLI_math_vector.hh
  KLI_memarena.h
  KLI_memory_utils.hh
  KLI_mempool.h
  KLI_numaapi.h
  KLI_path_utils.h
  KLI_probing_strategies.hh
  KLI_rect.h
  KLI_rhash.h
  KLI_set.hh
  KLI_set_slots.hh
  KLI_simd.h
  KLI_span.hh
  KLI_strict_flags.h
  KLI_string.h
  KLI_string_ref.hh
  KLI_string_utf8.h
  KLI_string_utils.h
  KLI_sys_types.h
  KLI_system.h
  KLI_task.h
  KLI_task.hh
  KLI_threads.h
  KLI_time.h
  KLI_utildefines.h
  KLI_utildefines_variadic.h
  KLI_vector.hh
  KLI_winstuff.h
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
  extern_wcwidth
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