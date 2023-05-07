set(INC
  .
  ./intern
  ../include
  ../../../../. # Pixar USD
  ../../../../source
  ../../python
  ../../gpu
  ../../anchor
  ../../imbuf
  ../../kraklib
  ../../krakernel
  ../../krakfont
  ../../krakloader
  ../../luxo
  ../../universe
  ../../wm
  ../../../../intern/atomic
  ../../../../intern/clog
  ../../../../intern/guardedalloc
  ${CMAKE_BINARY_DIR}/source/kraken/luxo
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
  INTERFACE_drag.cpp
  INTERFACE_handlers.cpp
  INTERFACE_icons.c
  INTERFACE_kraken.cpp
  INTERFACE_layout.cpp
  INTERFACE_ops.cpp
  INTERFACE_panel.cpp
  INTERFACE_query.cpp
  INTERFACE_region_menu_popup.cpp
  INTERFACE_region_popup.cpp
  INTERFACE_region_search.cpp
  INTERFACE_region_tooltip.cpp
  INTERFACE_regions.cpp
  INTERFACE_resources.c
  INTERFACE_style.cpp
  INTERFACE_templates.cpp
  INTERFACE_tokens.cpp
  INTERFACE_utils.cpp
  INTERFACE_view2d.cpp

  views/abstract_view_item.cpp

  intern/interface_intern.h
  intern/interface_regions_intern.h
)

set(LIB
  maelstrom
  maelstrom_static
  kraken_kernel
  kraken_lib
  kraken_editor_datafiles
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

kraken_add_lib(kraken_editor_interface "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")