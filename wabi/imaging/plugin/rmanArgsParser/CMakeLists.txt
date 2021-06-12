set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE rmanArgsParser)

wabi_plugin(rmanArgsParser
    LIBRARIES
        arch
        gf
        tf
        vt
        ar
        ndr
        sdr

    PRIVATE_CLASSES
        rmanArgsParser

    CPPFILES
        pugixml/pugixml.cpp
    
    PRIVATE_HEADERS
        api.h
        pugixml/pugiconfig.hpp
        pugixml/pugixml.hpp

    PYMODULE_CPPFILES
        module.cpp
        wrapRmanArgsParser.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
)
