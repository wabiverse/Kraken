add_subdirectory(garch)
add_subdirectory(hf)
add_subdirectory(hio)
add_subdirectory(cameraUtil)
add_subdirectory(pxOsd)
add_subdirectory(vkf)
add_subdirectory(glf)
add_subdirectory(hgi)
add_subdirectory(hgiGL)

if(APPLE)
  add_subdirectory(hgiMetal)
endif()

add_subdirectory(hgiVulkan)
add_subdirectory(hgiInterop)
add_subdirectory(hd)

if(WITH_MATERIALX)
  add_subdirectory(hdMtlx)
endif()

add_subdirectory(hdPh)
add_subdirectory(hdx)

if(WITH_PRORENDER)
  add_subdirectory(rprUsd)
endif()

add_subdirectory(plugin)
