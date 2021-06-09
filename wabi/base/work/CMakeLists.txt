set(WABI_PREFIX wabi/base)
set(WABI_PACKAGE work)

wabi_library(work
    LIBRARIES
        tf
        trace
        ${TBB_LIBRARIES}

    INCLUDE_DIRS
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        detachedTask
        dispatcher
        loops
        reduce
        singularTask
        threadLimits
        utils

    PUBLIC_HEADERS
        api.h
        withScopedParallelism.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapThreadLimits.cpp

    PYMODULE_FILES
        python/__init__.py

    DOXYGEN_FILES
        docs/overview.dox
)
