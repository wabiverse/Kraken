set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE rmanDiscovery)

wabi_plugin(rmanDiscovery
    LIBRARIES
        gf
        plug
        tf
        vt
        ar
        ndr
        sdr

    PRIVATE_CLASSES
        rmanDiscovery

    RESOURCE_FILES
        resources/plugInfo.json

    DISABLE_PRECOMPILED_HEADERS
)
