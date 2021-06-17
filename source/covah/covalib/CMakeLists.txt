set(INC
  .
  ../../../.
  ../editors/include
)

set(SRC
  intern/CLI_assert.c
  intern/CLI_string_utils.cpp
  intern/CLI_icons.cpp
  intern/CLI_math.c
  intern/CLI_time.cpp

  CLI_api.h
  CLI_assert.h
  CLI_compiler_attrs.h
  CLI_icons.h
  CLI_math_inline.h
  CLI_string_utils.h
  CLI_time.h
  CLI_utildefines.h
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
)

set(LIB
  covahverse
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

add_definitions(-DCOVAH_LIB_EXPORTS)
covah_add_lib(covah_lib "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")