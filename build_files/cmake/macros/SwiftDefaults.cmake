# Set swift to something very recent.
set(CMAKE_Swift_LANGUAGE_VERSION 5.9)

# Enable experiemental cxx in swift interoperabiliy.
set(CMAKE_Swift_FLAGS "${CMAKE_Swift_FLAGS} -cxx-interoperability-mode=default")

# Static linkage.
set(CMAKE_Swift_FLAGS "${CMAKE_Swift_FLAGS} -static")

# Output of swift modules.
set(CMAKE_Swift_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/swift CACHE STRING "Output directory of compiled Swift modules." FORCE)
set(Swift_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/swift CACHE STRING "Output directory of compiled Swift modules." FORCE)