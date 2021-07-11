set(INC
  .
  ../../../. # Pixar USD
  ../krakernel
  ../universe
  ../editors/include
)


if(WIN32)
  set(SRC_WINDOWS
    intern/KLI_system_win32.cpp
  )
endif()

set(SRC
  intern/KLI_assert.c
  intern/KLI_string_utils.cpp
  intern/KLI_icons.cpp
  intern/KLI_math.c
  intern/KLI_path_utils.cpp
  intern/KLI_time.cpp

  ${SRC_WINDOWS}

  KLI_api.h
  KLI_assert.h
  KLI_compiler_attrs.h
  KLI_icons.h
  KLI_math_inline.h
  KLI_path_utils.h
  KLI_string_utils.h
  KLI_time.h
  KLI_utildefines.h
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
)

list(APPEND LIB
  maelstrom
  kraken_kernel
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

if(WIN32)
  list(APPEND LIB
    version.lib
  )
endif()

add_definitions(-DKRAKEN_LIB_EXPORTS)
kraken_add_lib(kraken_lib "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")