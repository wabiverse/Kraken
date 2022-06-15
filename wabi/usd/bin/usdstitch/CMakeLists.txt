set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usd)

pxr_python_bin(usdstitch
    DEPENDENCIES
        tf
        sdf
        usdUtils
)
