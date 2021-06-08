set(WABI_PREFIX wabi/usdImaging)
set(WABI_PACKAGE usdRiImaging)

wabi_library(usdRiImaging
    LIBRARIES
        gf
        tf
        plug
        trace
        vt
        work
        hd
        pxOsd
        sdf
        usd
        usdGeom
        usdLux
        usdShade
        usdImaging
        usdVol
        ar
        ${TBB_LIBRARIES}

    INCLUDE_DIRS
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        pxrBarnLightFilterAdapter
        pxrIntMultLightFilterAdapter
        pxrRodLightFilterAdapter
        tokens

    PUBLIC_HEADERS
        api.h
        version.h

    RESOURCE_FILES
        resources/plugInfo.json
)
