# Set swift to something very recent.
set(CMAKE_Swift_LANGUAGE_VERSION 5.7)

# Enable experiemental cxx in swift interoperabiliy.
set(CMAKE_Swift_FLAGS "${CMAKE_Swift_FLAGS} -enable-experimental-cxx-interop")

# Static linkage.
set(CMAKE_Swift_FLAGS "${CMAKE_Swift_FLAGS} -static")