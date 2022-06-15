# It's important to not include the wrong math.h here.
set(CMAKE_INCLUDE_CURRENT_DIR OFF)

set(WABI_PREFIX wabi/base)
set(WABI_PACKAGE arch)

set(PLATFORM_LIBS ${M_LIB})
if(WIN32)
    set(PLATFORM_LIBS Ws2_32.lib Dbghelp.lib ${PLATFORM_LIBS})
endif()

wabi_library(arch
    LIBRARIES
        ${CMAKE_DL_LIBS}
        ${PLATFORM_LIBS}

    PUBLIC_CLASSES
        align
        attributes
        daemon
        debugger
        demangle
        env
        error
        errno
        fileSystem
        function
        hash
        library
        mallocHook
        regex
        stackTrace
        symbols
        systemInfo
        threads
        timing
        virtualMemory
        vsnprintf

    PUBLIC_HEADERS
        api.h
        buildMode.h
        defines.h
        export.h
        functionLite.h
        hints.h
        inttypes.h
        math.h
        pragmas.h

    PRIVATE_HEADERS
        testArchAbi.h
        testArchUtil.h

    CPPFILES
        assumptions.cpp
        initConfig.cpp

    DOXYGEN_FILES
        docs/overview.dox
)