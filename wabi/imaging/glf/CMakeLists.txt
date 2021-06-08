set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE glf)

if(NOT ${WITH_OPENGL})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_OPENGL is OFF")
  return()
endif()

set(optionalPublicClasses "")
if (X11_FOUND)
    list(APPEND optionalPublicClasses testGLContext)
endif()

set(optionalLibs "")
set(optionalIncludeDirs "")

wabi_library(glf
    LIBRARIES
        ar
        arch
        garch
        gf
        hf
        hio
        js
        plug
        tf
        trace
        sdf
        ${Boost_PYTHON_LIBRARY}
        ${OPENGL_gl_LIBRARY}
        ${X11_LIBRARIES}
        ${optionalLibs}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${optionalIncludeDirs}

    PUBLIC_CLASSES
        bindingMap
        contextCaps
        diagnostic
        drawTarget
        glContext
        glRawContext
        info
        simpleLight
        simpleLightingContext
        simpleMaterial
        simpleShadowArray
        texture
        uniformBlock
        utils
        ${optionalPublicClasses}

    PRIVATE_CLASSES
        debugCodes
        glContextRegistry

    PUBLIC_HEADERS
        api.h

    CPPFILES
        ${optionalCppFiles}

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapDiagnostic.cpp
        wrapDrawTarget.cpp
        wrapSimpleLight.cpp
        wrapSimpleMaterial.cpp
        wrapTexture.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/shaders/pcfShader.glslfx
        resources/shaders/simpleLighting.glslfx

    DOXYGEN_FILES
        docs/overview.dox
)
