set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usd)

wabi_python_bin(usddumpcrate
    LIBRARIES
      sdf
      usd
)
