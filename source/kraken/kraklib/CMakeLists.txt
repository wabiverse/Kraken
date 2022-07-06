set(INC
  .
  ../../../. # Pixar USD
  ../krakernel
  ../universe
  ../editors/include
)

set(SRC
  intern/KLI_assert.c
  intern/KLI_filelist.cpp
  intern/KLI_fileops.cpp
  intern/KLI_icons.cpp
  intern/KLI_math.c
  intern/KLI_numaapi.cpp
  intern/KLI_path_utils.cpp
  intern/KLI_storage.cpp
  intern/KLI_string_utils.cpp
  intern/KLI_time.cpp
  intern/KLI_threads.cpp
  intern/KLI_winstuff.cpp

  KLI_api.h
  KLI_assert.h
  KLI_compiler_attrs.h
  KLI_fileops.h
  KLI_icons.h
  KLI_math_inline.h
  KLI_numaapi.h
  KLI_path_utils.h
  KLI_string_utils.h
  KLI_threads.h
  KLI_time.h
  KLI_utildefines.h
  KLI_winstuff.h
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${PTHREADS_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIR}
)

list(APPEND LIB
  maelstrom
  maelstrom_static
  kraken_kernel
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${PTHREADS_LIBRARY}
  ${ZLIB_LIBRARY}
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