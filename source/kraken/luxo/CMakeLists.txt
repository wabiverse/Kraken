set(INC
  .
  ../../../. # Pixar USD
  ../editors/include
  ../universe
  ../krakernel
  ../kraklib
  ../wm
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
)

set(SRC
  intern/LUXO_access.cpp
  intern/LUXO_define.cpp
  intern/LUXO_main.cpp
  intern/LUXO_wm.cpp

  intern/LUXO_internal.h

  LUXO_access.h
  LUXO_define.h
  LUXO_main.h
  LUXO_runtime.h
)

set(LIB
  maelstrom
  maelstrom_static
  kraken_lib
  kraken_kernel
  kraken_universe
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

kraken_add_lib(kraken_luxo "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")