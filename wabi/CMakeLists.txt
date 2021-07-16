
#-------------------------------------------------------------------------------
# Setup our Pixar API.



wabi_core_prologue()

add_subdirectory(base)
add_subdirectory(usd)

if(${WABI_BUILD_IMAGING})
  add_subdirectory(imaging)
  if(${WABI_BUILD_USD_IMAGING})
    add_subdirectory(usdImaging)
  endif()
endif()

wabi_core_epilogue()



#-----------------------------------------------------------------------------
# Create the configuration target.



export(PACKAGE wabi)

configure_file(
  ${CMAKE_SOURCE_DIR}/build_files/cmake/config/wabiConfig.cmake.in
  "${PROJECT_BINARY_DIR}/wabiConfig.cmake" @ONLY)

if(UNIX)
  install(FILES
    "${PROJECT_BINARY_DIR}/wabiConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_PREFIX}"
  )
elseif(WIN32)
  install(FILES
    "${PROJECT_BINARY_DIR}/wabiConfig.cmake"
    DESTINATION "./cmake/wabiConfig.cmake"
  )
endif()

if(NOT WITH_KRAKEN_MONOLITHIC)
  install(EXPORT wabiTargets DESTINATION "cmake")
endif()
