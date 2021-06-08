# Don't get the wrong half.h!
set(CMAKE_INCLUDE_CURRENT_DIR OFF)

set(WABI_PREFIX wabi/base)
set(WABI_PACKAGE gf)

wabi_library(gf
    LIBRARIES
        arch
        tf

    PUBLIC_CLASSES
        bbox3d
        camera
        frustum
        gamma
        half
        homogeneous
        ilmbase_half
        interval
        line
        line2d
        lineSeg
        lineSeg2d
        math
        matrixData
        matrix2d
        matrix2f
        matrix3f
        matrix3d
        matrix4f
        matrix4d
        multiInterval
        plane
        quatd
        quatf
        quath
        quaternion
        range1d
        range1f
        range2d
        range2f
        range3d
        range3f
        ray
        rect2i
        rotation
        size2
        size3
        transform
        vec2d
        vec2f
        vec2h
        vec2i
        vec3d
        vec3f
        vec3h
        vec3i
        vec4d
        vec4f
        vec4h
        vec4i

    PYTHON_PUBLIC_CLASSES
        pyBufferUtils

    PUBLIC_HEADERS
        api.h
        declare.h
        ilmbase_halfLimits.h
        limits.h
        traits.h

    PRIVATE_CLASSES
        ostreamHelpers

    PRIVATE_HEADERS
        ilmbase_eLut.h
        ilmbase_toFloat.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapBBox3d.cpp
        wrapCamera.cpp
        wrapFrustum.cpp
        wrapGamma.cpp
        wrapHalf.cpp
        wrapHomogeneous.cpp
        wrapInterval.cpp
        wrapLimits.cpp
        wrapLine.cpp
        wrapLineSeg.cpp
        wrapMath.cpp
        wrapMatrix2d.cpp
        wrapMatrix2f.cpp
        wrapMatrix3f.cpp
        wrapMatrix3d.cpp
        wrapMatrix4f.cpp
        wrapMatrix4d.cpp
        wrapMultiInterval.cpp
        wrapPlane.cpp
        wrapQuaternion.cpp
        wrapQuatd.cpp
        wrapQuatf.cpp
        wrapQuath.cpp
        wrapRange1d.cpp
        wrapRange1f.cpp
        wrapRange2d.cpp
        wrapRange2f.cpp
        wrapRange3d.cpp
        wrapRange3f.cpp
        wrapRay.cpp
        wrapRect2i.cpp
        wrapRotation.cpp
        wrapSize2.cpp
        wrapSize3.cpp
        wrapTransform.cpp
        wrapVec2d.cpp
        wrapVec2f.cpp
        wrapVec2h.cpp
        wrapVec2i.cpp
        wrapVec3d.cpp
        wrapVec3f.cpp
        wrapVec3h.cpp
        wrapVec3i.cpp
        wrapVec4d.cpp
        wrapVec4f.cpp
        wrapVec4h.cpp
        wrapVec4i.cpp

    PYMODULE_FILES
        python/__init__.py

    DOXYGEN_FILES
        docs/overview.dox
)

# gfGenCode --validate ensures that the generated GfVec, GfMatrix, GfRange, etc.
# source files are up to date with the contents of the templates used to
# generate them. If not, a diff displaying the difference will be printed to
# stdout and the build will fail.
if (NOT ${WABI_VALIDATE_GENERATED_CODE})
    message(STATUS "Skipping validation of gf generated code because WABI_VALIDATE_GENERATED_CODE=OFF")
elseif(NOT JINJA2_FOUND)
    message(STATUS "Skipping validation of gf generated code because of missing dependency: jinja2")
else()
    add_custom_command(OUTPUT gfverify.out
        DEPENDS gf
        PRE_BUILD
        COMMENT "Validating gf generated code..."
        COMMAND
            ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/python/gfGenCode.py
                --srcDir ${CMAKE_CURRENT_SOURCE_DIR}
                --validate
    )
endif()
