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
  intern/LUXO_wm.cpp

  LUXO_access.h
  LUXO_runtime.h
)

set(LIB
  maelstrom
  kraken_lib
  kraken_kernel
  kraken_universe
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

kraken_import_nuget_packages("source/kraken/luxo/kraken_luxo")
kraken_add_lib(kraken_luxo "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")