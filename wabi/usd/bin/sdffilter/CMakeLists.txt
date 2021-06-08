set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usd)

wabi_cpp_bin(sdffilter
    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}

    LIBRARIES
        arch
        tf
        sdf
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
