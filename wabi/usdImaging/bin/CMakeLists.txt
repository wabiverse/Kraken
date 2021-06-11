# add_subdirectory(usdrecord)
# add_subdirectory(testusdview)

if(WITH_PIXAR_USDVIEW)
  install(
    FILES
      usdview.py
    DESTINATION
      /usr/local/share/covah/${TARGETDIR_VER}/bin/toolkit)
endif()
