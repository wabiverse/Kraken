set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE RtxHioImage)

wabi_plugin(${WABI_PACKAGE}

    LIBRARIES
        tf
        glf
        ${PXRCORE_LIBRARY}
        ${PRMAN_LIBRARY}

    INCLUDE_DIRS
        ${GLEW_INCLUDE_DIR}    
        ${RENDERMAN_INCLUDE_DIR}

    CPPFILES
        glfimage.cpp 

    PRIVATE_HEADERS

    RESOURCE_FILES

    DISABLE_PRECOMPILED_HEADERS
)
