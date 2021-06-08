set(WABI_PREFIX wabi/base)
set(WABI_PACKAGE vt)

wabi_library(vt
    LIBRARIES
        arch
        tf
        gf
        trace
        ${Boost_PYTHON_LIBRARY}
        ${TBB_LIBRARIES}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        array
        dictionary
        functions
        hash
        streamOut
        types
        value

    PYTHON_PUBLIC_CLASSES
        arrayPyBuffer
        valueFromPython
        wrapArray

    PUBLIC_HEADERS
        api.h
        traits.h
        operators.h

    PYTHON_PUBLIC_HEADERS
        pyOperators.h

    PRIVATE_HEADERS
        typeHeaders.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapArrayBase.cpp
        wrapArrayFloat.cpp
        wrapArrayIntegral.cpp
        wrapArrayMatrix.cpp
        wrapArrayQuaternion.cpp
        wrapArrayRange.cpp
        wrapArrayString.cpp
        wrapArrayToken.cpp
        wrapArrayVec.cpp
        wrapDictionary.cpp
        wrapValue.cpp

    PYMODULE_FILES
        python/__init__.py

    DOXYGEN_FILES
        docs/overview.dox
)
