set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hdx)

# XXX Ensure Phoenix uses only Hgi, for cross platform
#     compatibility with Vulkan & Metal & OpenGL.
if(NOT ${WITH_GPU_SUPPORT})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT is OFF")
  return()
endif()

set (optionalCppFiles "")
if (WITH_OPENCOLORIO)
    list(APPEND optionalLibs ${OCIO_LIBRARIES})
    list(APPEND optionalIncludeDirs ${OCIO_INCLUDE_DIRS})
endif()

wabi_library(hdx
    LIBRARIES
        plug
        tf
        vt
        gf
        work
        garch
        glf
        pxOsd
        hd
        hdSt
        hgi
        hgiInterop
        cameraUtil
        sdf
        ${optionalLibs}

    INCLUDE_DIRS
        ${optionalIncludeDirs}

    PUBLIC_CLASSES
        aovInputTask
        colorizeSelectionTask
        colorChannelTask
        colorCorrectionTask
        drawTargetTask
        freeCameraSceneDelegate
        fullscreenShader
        hgiConversions
        oitBufferAccessor
        oitRenderTask
        oitResolveTask
        oitVolumeRenderTask
        package
        pickTask
        pickFromRenderBufferTask
        presentTask
        renderTask
        renderSetupTask
        selectionTask
        selectionTracker
        shadowTask
        simpleLightTask
        skydomeTask
        task
        taskController
        tokens
        types
        visualizeAovTask

    PUBLIC_HEADERS
        shadowMatrixComputation.h
        version.h
        api.h

    PRIVATE_CLASSES
        debugCodes
        unitTestDelegate

    PRIVATE_HEADERS
        unitTestUtils.h

    RESOURCE_FILES
        resources/plugInfo.json
        resources/shaders/colorChannel.glslfx
        resources/shaders/colorCorrection.glslfx
        resources/shaders/fullscreen.glslfx
        resources/shaders/oitResolveImageShader.glslfx
        resources/shaders/outline.glslfx
        resources/shaders/renderPass.glslfx
        resources/shaders/renderPassColorShader.glslfx
        resources/shaders/renderPassColorAndSelectionShader.glslfx
        resources/shaders/renderPassColorWithOccludedSelectionShader.glslfx
        resources/shaders/renderPassIdShader.glslfx
        resources/shaders/renderPassPickingShader.glslfx
        resources/shaders/renderPassOitShader.glslfx
        resources/shaders/renderPassOitOpaqueShader.glslfx
        resources/shaders/renderPassOitVolumeShader.glslfx
        resources/shaders/renderPassShadowShader.glslfx
        resources/shaders/selection.glslfx
        resources/shaders/visualize.glslfx
        resources/textures/StinsonBeach.exr

    DOXYGEN_FILES
        docs/overview.dox
)
