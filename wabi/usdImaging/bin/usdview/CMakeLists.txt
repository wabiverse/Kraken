set(WABI_PREFIX wabi/usdImaging)
set(WABI_PACKAGE usdviewq)

if (NOT WABI_BUILD_USDVIEW)
    return()
endif()

wabi_python_bin(usdview
    DEPENDENCIES
        usdviewq
)
