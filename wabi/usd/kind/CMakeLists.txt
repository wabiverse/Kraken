set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE kind)

wabi_library(kind
    LIBRARIES
        tf
        plug

    PUBLIC_CLASSES
        registry
        
    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapRegistry.cpp
        wrapTokens.cpp

    PYMODULE_FILES 
        python/__init__.py

    DOXYGEN_FILES
        docs/overview.dox
)
