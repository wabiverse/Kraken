set(INC
  .
  ../../../. # Pixar USD
  ../anchor
  ../anchor/intern
  ../anchor/backend
  ../editors/include
  ../kraklib
  ../wm
  ../universe
  ../gatekeeper
  ../python
)

set(SRC
  intern/KKE_context.cpp
  intern/KKE_kraken.cpp
  intern/KKE_diagnostics.cpp
  intern/KKE_hash.cpp
  intern/KKE_icons.cpp
  intern/KKE_plugins.cpp
  intern/KKE_python.cpp
  intern/KKE_screen.cpp
  intern/KKE_utils.cpp
  intern/KKE_workspace.cpp

  KKE_api.h
  KKE_context.h
  KKE_icons.h
  KKE_main.h
  KKE_robinhood.h
  KKE_screen.h
  KKE_space.h
  KKE_workspace.h
  KKE_utils.h
  KKE_version.h
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
  maelstrom
  kraken_anchor
  kraken_lib
  kraken_python
  kraken_wm
  kraken_universe
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${PTHREADS_LIBRARY}
)

add_definitions(-DKRAKEN_KERNEL_EXPORTS)
kraken_add_lib(kraken_kernel "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")