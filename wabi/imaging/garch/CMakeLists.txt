set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE garch)

# Garch :: Graphics Architecture -- Provides abstractions for
# architecture-specific functionality that may pull in imaging
# related dependencies. And will need to be agnostic of GFX
# libraries such as Vulkan & OpenGL & Metal. 
if(NOT ${WITH_GPU_SUPPORT})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT is OFF")
  return()
endif()

if(APPLE)
    set(GARCH_GLPLATFORMCONTEXT       glPlatformContextDarwin)
    set(GARCH_GLPLATFORMDEBUGWINDOW   glPlatformDebugWindowDarwin)
    set(GARCH_SOURCE_EXTENSION        mm)
    FIND_LIBRARY(APPKIT_LIBRARY AppKit)
elseif(X11_FOUND)
    set(GARCH_GLPLATFORMCONTEXT       glPlatformContextGLX)
    set(GARCH_GLPLATFORMDEBUGWINDOW   glPlatformDebugWindowGLX)
    set(GARCH_SOURCE_EXTENSION        cpp)
elseif(WIN32)
    set(GARCH_GLPLATFORMCONTEXT       glPlatformContextWindows)
    set(GARCH_GLPLATFORMDEBUGWINDOW   glPlatformDebugWindowWindows)
    set(GARCH_SOURCE_EXTENSION        cpp)
endif()

wabi_library(garch
    LIBRARIES
        arch
        tf
        ${X11_LIBRARIES}
        ${OPENGL_gl_LIBRARY}
        ${APPKIT_LIBRARY}
        ${VULKAN_LIBRARIES}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}

    PUBLIC_CLASSES
        glApi
        glDebugWindow
        glPlatformDebugContext

    PUBLIC_HEADERS
        api.h
        gl.h
        glPlatformContext.h
        khrplatform.h
        ${GARCH_GLPLATFORMCONTEXT}.h

    PRIVATE_HEADERS
        ${GARCH_GLPLATFORMDEBUGWINDOW}.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    CPPFILES
        ${GARCH_GLPLATFORMCONTEXT}.${GARCH_SOURCE_EXTENSION}
        ${GARCH_GLPLATFORMDEBUGWINDOW}.${GARCH_SOURCE_EXTENSION}

    PYMODULE_CPPFILES
        module.cpp
        wrapPlatformDebugContext.cpp

    PYMODULE_FILES
        python/__init__.py
)
