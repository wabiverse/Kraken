set(WABI_PREFIX wabi/usdImaging)
set(WABI_PACKAGE usdVolImaging)

wabi_library(usdVolImaging
    LIBRARIES
        usdImaging

    PUBLIC_CLASSES
        openvdbAssetAdapter
        field3dAssetAdapter
        tokens

    PUBLIC_HEADERS
        api.h

    RESOURCE_FILES
        resources/plugInfo.json
)
