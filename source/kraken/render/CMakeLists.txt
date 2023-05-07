set(INC
  .
  intern
  ../../../. # Pixar USD
  ../../../source
  ../krakernel
  ../kraklib
  # ../hydra
  ../draw
  ../gpu
  ../imbuf
  ../universe
  ../luxo
  # ../nodes
  # ../sequencer
  # ../simulation
  ../wm
  ../../../intern/atomic
  ../../../intern/guardedalloc
  # ../../../intern/mikktspace
  # ../../../intern/mantaflow/extern
)


set(SRC
  intern/engine.cc

  RE_engine.h
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIR}
  ${Vulkan_INCLUDE_DIRS}
  ${PTHREADS_INCLUDE_DIR}
)

set(LIB
)

if(WITH_PYTHON)
  add_definitions(-DWITH_PYTHON)
  list(APPEND INC
    ../python
  )
endif()

# if(WITH_IMAGE_OPENEXR)
#   list(APPEND LIB
#     kraken_imbuf_openexr
#   )
#   add_definitions(-DWITH_OPENEXR)
# endif()

# if(WITH_FREESTYLE)
#   list(APPEND INC
#     ../freestyle
#   )
#   list(APPEND LIB
#     kraken_freestyle
#   )
#   add_definitions(-DWITH_FREESTYLE)
# endif()


kraken_add_lib_nolist(kraken_render "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
