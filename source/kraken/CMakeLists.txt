# kraken lib depends on eigen.
add_subdirectory(eigen)

add_subdirectory(KrakenOS)
add_subdirectory(anchor)
add_subdirectory(editors)
add_subdirectory(wm)
add_subdirectory(krakernel)
add_subdirectory(kraklib)
add_subdirectory(gpu)
add_subdirectory(imbuf)
add_subdirectory(universe)
add_subdirectory(luxo)
add_subdirectory(python)

# if(WITH_TESTS)
#   add_subdirectory(test)
# endif()