set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE cameraUtil)

wabi_library(cameraUtil
    LIBRARIES
        tf
        gf

    PUBLIC_CLASSES
        conformWindow
        framing
        screenWindowParameters

    PYTHON_CPPFILES
        moduleDeps.cpp

    PUBLIC_HEADERS
        api.h

    PYMODULE_CPPFILES
        module.cpp
        wrapConformWindow.cpp
        wrapFraming.cpp
        wrapScreenWindowParameters.cpp

    PYMODULE_FILES
        python/__init__.py
)
