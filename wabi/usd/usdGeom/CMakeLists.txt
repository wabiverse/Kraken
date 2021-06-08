set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdGeom)

wabi_library(usdGeom
    LIBRARIES
        js
        tf
        plug
        vt
        sdf
        ndr
        gdr
        trace
        usd
        work
        ${Boost_ATOMIC_LIBRARY}
        ${Boost_PYTHON_LIBRARY}
        ${TBB_LIBRARIES}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        connectableAPI
        connectableAPIBehavior
        debugCodes
        tokens
        bboxCache
        constraintTarget
        xformCache
        basisCurves
        boundable
        boundableComputeExtent
        camera
        capsule
        cone
        cube
        curves
        cylinder
        grid
        hermiteCurves
        imageable
        input
        gprim
        mesh
        metrics
        modelAPI
        motionAPI
        nodeDefAPI
        nodeGraph
        nurbsCurves
        nurbsPatch
        output
        pointBased
        pointInstancer
        points
        primvar
        primvarsAPI
        scope
        sphere
        subset
        suzanne
        utils
        xform
        xformable
        xformOp
        xformCommonAPI

    PUBLIC_HEADERS
        api.h
        types.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PRIVATE_CLASSES
        samplingUtils

    PYMODULE_CPPFILES
        module.cpp
        wrapBBoxCache.cpp
        wrapBasisCurves.cpp
        wrapBoundable.cpp
        wrapCamera.cpp
        wrapConstraintTarget.cpp
        wrapCapsule.cpp
        wrapCone.cpp
        wrapCube.cpp
        wrapCurves.cpp
        wrapCylinder.cpp
        wrapGprim.cpp
        wrapHermiteCurves.cpp
        wrapImageable.cpp
        wrapMesh.cpp
        wrapMetrics.cpp
        wrapModelAPI.cpp
        wrapMotionAPI.cpp
        wrapNurbsCurves.cpp
        wrapNurbsPatch.cpp
        wrapPointBased.cpp
        wrapPointInstancer.cpp
        wrapPoints.cpp
        wrapPrimvar.cpp
        wrapPrimvarsAPI.cpp
        wrapScope.cpp
        wrapSphere.cpp
        wrapSubset.cpp
        wrapTokens.cpp
        wrapXform.cpp
        wrapXformCache.cpp
        wrapXformable.cpp
        wrapXformOp.cpp
        wrapXformCommonAPI.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/generatedSchema.usda
        resources/schema.usda:usdGeom/schema.usda

    DOXYGEN_FILES
        docs/overview.dox
        docs/snippets.dox
)
