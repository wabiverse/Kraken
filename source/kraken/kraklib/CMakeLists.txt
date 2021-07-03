set(INC
  .
  ../../../.
  ../editors/include
)

set(SRC
  intern/KLI_assert.c
  intern/KLI_string_utils.cpp
  intern/KLI_icons.cpp
  intern/KLI_math.c
  intern/KLI_path_utils.cpp
  intern/KLI_time.cpp

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

set(LIB
  krakenverse
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

add_definitions(-DKRAKEN_LIB_EXPORTS)
kraken_add_lib(kraken_lib "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")