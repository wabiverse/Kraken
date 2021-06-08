set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usd)

wabi_python_bin(usdstitch
    DEPENDENCIES
        tf
        sdf
        usdUtils
)
