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
  intern/CKE_covah.cpp
  intern/CKE_diagnostics.cpp
  intern/CKE_plugins.cpp
  intern/CKE_python.cpp
  intern/CKE_utils.cpp

  CKE_api.h
  CKE_context.h
  CKE_main.h
  CKE_space.h
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
  covahverse
  covah_anchor
  covah_lib
  covah_python
  covah_wm
  covah_universe
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${Vulkan_LIBRARIES}
)

add_definitions(-DCOVAH_KERNEL_EXPORTS)
covah_add_lib(covah_kernel "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")