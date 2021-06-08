set(WABI_PREFIX wabi/usdImaging)
set(WABI_PACKAGE usdSkelImaging)

wabi_library(usdSkelImaging
    LIBRARIES
        hio
        hd
        usdImaging
        usdSkel

    PUBLIC_CLASSES
        skeletonAdapter
        skelRootAdapter
        package
        utils

    PUBLIC_HEADERS
        api.h

    RESOURCE_FILES
        resources/plugInfo.json
        resources/shaders/skinning.glslfx
)
