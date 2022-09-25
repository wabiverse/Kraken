set(INC
  .
  ./intern
  ../include/.
  ../../../../. # Pixar USD
  ../../../../source
  ../../../../intern/guardedalloc
  ../../python/kpy
  ../../gpu
  ../../anchor
  ../../imbuf
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

set(SRC
  INTERFACE_align.cpp
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
  INTERFACE_style.cpp
  INTERFACE_tokens.cpp
  INTERFACE_view2d.cpp

  views/abstract_view_item.cpp

  intern/interface_intern.h
  intern/interface_regions_intern.h
)

set(LIB
  maelstrom
  maelstrom_static
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

kraken_add_lib(kraken_editor_interface "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")