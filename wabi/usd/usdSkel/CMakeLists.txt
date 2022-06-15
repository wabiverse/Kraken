set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdSkel)

wabi_library(usdSkel
    LIBRARIES
        arch
        gf
        tf
        trace
        vt
        work
        sdf
        usd
        usdGeom
        ${Boost_PYTHON_LIBRARY}
        ${TBB_tbb_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        animation
        animMapper
        animQuery
        bakeSkinning
        bindingAPI
        blendShape
        blendShapeQuery
        cache
        debugCodes
        inbetweenShape
        packedJointAnimation
        root
        skeleton
        skeletonQuery
        skinningQuery
        tokens
        topology
        utils

    PUBLIC_HEADERS
        api.h
        binding.h

    PRIVATE_CLASSES
        animQueryImpl
        cacheImpl
        skelDefinition

    PYTHON_CPPFILES
        moduleDeps.cpp    

    PYMODULE_FILES
        python/__init__.py

    PYMODULE_CPPFILES
        module.cpp
        wrapAnimation.cpp
        wrapAnimMapper.cpp
        wrapAnimQuery.cpp
        wrapBakeSkinning.cpp
        wrapBlendShapeQuery.cpp
        wrapBinding.cpp
        wrapBindingAPI.cpp
        wrapBlendShape.cpp
        wrapCache.cpp
        wrapInbetweenShape.cpp
        wrapPackedJointAnimation.cpp
        wrapRoot.cpp
        wrapSkeleton.cpp
        wrapSkeletonQuery.cpp
        wrapSkinningQuery.cpp
        wrapTokens.cpp
        wrapTopology.cpp
        wrapUtils.cpp

    RESOURCE_FILES
        resources/generatedSchema.usda
        resources/plugInfo.json
        resources/schema.usda:usdSkel/schema.usda

    DOXYGEN_FILES
        overview.dox
        docs/apiIntro.dox
        docs/bestPractices.dox
        docs/instancing.dox
        docs/intro.dox
        docs/objectModel.dox
        docs/schemaOverview.dox
        docs/schemas.dox
        docs/skinnedArm.usda
        docs/images/blendshapes.svg
        docs/images/blendshapesWithInbetweens.svg
        docs/images/influencesPrimvarLayout.svg
        docs/images/unboundedInterpolationExample.svg
)
