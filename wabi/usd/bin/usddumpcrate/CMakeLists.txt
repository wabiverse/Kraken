set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usd)

pxr_python_bin(usddumpcrate
    LIBRARIES
      sdf
      usd
)
