add_subdirectory(usdImaging)

if(WITH_VULKAN)
  add_subdirectory(usdApollo)
endif()

add_subdirectory(usdImagingGL)
add_subdirectory(usdRiImaging)
add_subdirectory(usdSkelImaging)
add_subdirectory(usdVolImaging)
add_subdirectory(usdAppUtils)
add_subdirectory(usdviewq)
add_subdirectory(plugin)
add_subdirectory(bin)