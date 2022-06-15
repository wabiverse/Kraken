set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hf)

set(optionalPublicClasses "")

wabi_library(hf
    LIBRARIES
        plug
        tf
        trace

    PUBLIC_HEADERS
        api.h
        diagnostic.h
        pluginDesc.h
        perfLog.h

    PUBLIC_CLASSES
        pluginBase
        pluginRegistry
        
    PRIVATE_CLASSES
        pluginEntry
)
