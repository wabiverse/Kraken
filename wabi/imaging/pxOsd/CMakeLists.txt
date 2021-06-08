set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE pxOsd)

wabi_library(pxOsd
    LIBRARIES
        tf
        gf
        vt
        ${OPENSUBDIV_LIBRARIES}
        ${Boost_PYTHON_LIBRARY}

    INCLUDE_DIRS
        ${OPENSUBDIV_INCLUDE_DIRS}

    PUBLIC_HEADERS
        api.h

    PUBLIC_CLASSES
        meshTopology
        meshTopologyValidation
        refinerFactory
        subdivTags
        tokens

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapMeshTopology.cpp
        wrapMeshTopologyValidation.cpp
        wrapSubdivTags.cpp
        wrapTokens.cpp

    PYMODULE_FILES
        python/__init__.py
)

