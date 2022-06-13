# add_subdirectory(usdrecord)
# add_subdirectory(testusdview)

if(WITH_PIXAR_USDVIEW)
  if(UNIX AND NOT APPLE)
    install(
      FILES
        usdview.py
      DESTINATION
        /usr/local/share/kraken/${TARGETDIR_VER}/bin/toolkit)
  elseif(APPLE)
    install(
      FILES
        usdview.py
      DESTINATION
        ${TARGETDIR_VER}/toolkit)
  elseif(WIN32)
    install(
      FILES
        usdview.py
      DESTINATION
        ${TARGETDIR_VER}/toolkit)
  endif()
endif()
