set(INC
  .
  ../../../. # Pixar USD
  ../../../source
  ../krakfont
  ../krakernel
  ../kraklib
  ../draw
  ../editors/include
  ../imbuf
  ../universe
  ../luxo
  ../wm
  ../../../intern/clog
  ../../../intern/guardedalloc

  ${CMAKE_BINARY_DIR}/source/kraken/luxo/intern
  ${CMAKE_BINARY_DIR}/source/kraken/luxo
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
  ${ZSTD_INCLUDE_DIRS}
)

set(SRC
  intern/readfile.cc
  intern/versioning_defaults.cc

  KLO_readfile.h
)

set(LIB
  maelstrom
  maelstrom_static
  kraken_kernel
  kraken_lib
)

kraken_add_lib(kraken_loader "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

add_dependencies(kraken_loader kraken_universe)
add_dependencies(kraken_loader kraken_luxo)

