set(INC
  .
  ../include
  ../../../../. # Pixar USD
  ../../../../source
  ../../krakernel
  ../../kraklib
  ../../gpu
  ../../universe
  ../../luxo
  ../../wm
  ../../../../intern/guardedalloc
)

set(INC_SYS

)

set(SRC
  spacetypes.c
)

set(LIB
  # kraken_editor_geometry
  # kraken_editor_space_action
  # kraken_editor_space_buttons
  # kraken_editor_space_clip
  # kraken_editor_space_console
  # kraken_editor_space_file
  # kraken_editor_space_graph
  # kraken_editor_space_image
  # kraken_editor_space_info
  # kraken_editor_space_nla
  # kraken_editor_space_node
  # kraken_editor_space_outliner
  # kraken_editor_space_script
  # kraken_editor_space_sequencer
  # kraken_editor_space_spreadsheet
  # kraken_editor_space_statusbar
  # kraken_editor_space_text
  # kraken_editor_space_topbar
  kraken_editor_space_userpref
  kraken_editor_space_view3d
)

kraken_add_lib(kraken_editor_space_api "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
