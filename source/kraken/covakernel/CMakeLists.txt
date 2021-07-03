set(INC
  .
  ../../../.
  ../anchor
  ../anchor/intern
  ../anchor/backend
  ../editors/include
  ../covalib
  ../wm
  ../universe
  ../gatekeeper
  ../python
)

set(SRC
  intern/CKE_context.cpp
  intern/CKE_kraken.cpp
  intern/CKE_diagnostics.cpp
  intern/CKE_hash.cpp
  intern/CKE_icons.cpp
  intern/CKE_plugins.cpp
  intern/CKE_python.cpp
  intern/CKE_screen.cpp
  intern/CKE_utils.cpp
  intern/CKE_workspace.cpp

  CKE_api.h
  CKE_context.h
  CKE_icons.h
  CKE_main.h
  CKE_robinhood.h
  CKE_screen.h
  CKE_space.h
  CKE_workspace.h
  CKE_utils.h
  CKE_version.h
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIR}
  ${Vulkan_INCLUDE_DIRS}
)

set(LIB
  krakenverse
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
)

add_definitions(-DKRAKEN_KERNEL_EXPORTS)
kraken_add_lib(kraken_kernel "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")