set(INC
  .
  ./intern
  ../include/.
  ../../../../. # Pixar USD
  ../../../../source
  ../../anchor
  ../../kraklib
  ../../krakernel
  ../../luxo
  ../../universe
  ../../wm
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIRS}
)

set(SRC+
  INTERFACE_button_group.cpp
  INTERFACE_handlers.cpp
  INTERFACE_kraken.cpp
  INTERFACE_layout.cpp
  INTERFACE_query.cpp
  INTERFACE_region_menu_popup.cpp
  INTERFACE_region_popup.cpp
  INTERFACE_region_search.cpp
  INTERFACE_region_tooltip.cpp
  INTERFACE_regions.cpp
  INTERFACE_tokens.cpp

  intern/interface_intern.h
  intern/interface_regions_intern.h
  ../include/UI_icons.h
  ../include/UI_interface.h
  ../include/UI_resources.h
  ../include/UI_tokens.h
)

set(LIB
  maelstrom
  maelstrom_static
  kraken_anchor
  kraken_kernel
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

kraken_add_lib(kraken_editor_interface "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")