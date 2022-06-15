add_subdirectory(hdStorm)
add_subdirectory(hdEmbree)
add_subdirectory(hioOiio)

if(WITH_ARNOLD)
  add_subdirectory(hdArnold)
endif()

if(WITH_CYCLES)
  add_subdirectory(hdCycles)
endif()

if(WITH_RENDERMAN)
  add_subdirectory(hdPrmanLoader)
  add_subdirectory(hdPrman)
  add_subdirectory(rmanArgsParser)
  add_subdirectory(rmanDiscovery)
  add_subdirectory(rmanOslParser)
  add_subdirectory(rtx_glfImage)
endif()

if(WITH_PRORENDER)
  add_subdirectory(hdRpr)
endif()
