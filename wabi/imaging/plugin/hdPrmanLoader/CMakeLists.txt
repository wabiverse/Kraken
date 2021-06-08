set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hdPrmanLoader)

wabi_plugin(hdPrmanLoader
   LIBRARIES
        arch
        plug
        tf
        hd

    PUBLIC_CLASSES
        rendererPlugin

    RESOURCE_FILES
        resources/plugInfo.json

    DISABLE_PRECOMPILED_HEADERS
)
