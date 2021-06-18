set(INC
  .
  ../../../.
  ../editors/include
  ../covakernel
  ../covalib
  ../wm
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
)

set(SRC
  intern/UNI_create.cpp
  intern/UNI_diagnostics.cpp

  UNI_api.h
  UNI_context.h
  UNI_diagnostics.h
  UNI_object.h
  UNI_path_defaults.h
  UNI_scene.h
  UNI_screen.h
  UNI_system.h
  UNI_userpref.h
  UNI_window.h
  UNI_workspace.h
)

set(LIB
  covahverse
  covah_lib
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

add_definitions(-DCOVAH_UNIVERSE_EXPORTS)
covah_add_lib(covah_universe "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")