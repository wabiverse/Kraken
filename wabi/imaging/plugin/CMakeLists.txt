add_subdirectory(hdPhoenix)
add_subdirectory(hdEmbree)
add_subdirectory(hdPrmanLoader)
add_subdirectory(hioOiio)
add_subdirectory(hioOpenVDB)

if(WITH_ARNOLD)
  add_subdirectory(hdArnold)
endif()

if(WITH_CYCLES)
  add_subdirectory(hdCycles)
endif()

if(WITH_PRORENDER)
  add_subdirectory(hdRpr)
endif()
