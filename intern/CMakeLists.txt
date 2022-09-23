add_subdirectory(atomic)
add_subdirectory(guardedalloc)

if(WIN32)
  add_subdirectory(utfconv)
endif()