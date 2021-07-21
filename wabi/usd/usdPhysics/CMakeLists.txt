set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdPhysics)

wabi_library(usdPhysics
    LIBRARIES
        js
        tf
        plug
        vt
        sdf
        trace
        usd
        usdGeom
        work
        ${Boost_PYTHON_LIBRARY}
        ${TBB_LIBRARIES}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        articulationRootAPI
        collisionAPI
        collisionGroup
        distanceJoint
        driveAPI
        filteredPairsAPI
        fixedJoint
        joint
        limitAPI
        massAPI
        materialAPI
        meshCollisionAPI
        metrics
        prismaticJoint
        revoluteJoint
        rigidBodyAPI
        scene
        sphericalJoint
        tokens        
        
    PUBLIC_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapArticulationRootAPI.cpp
        wrapCollisionAPI.cpp
        wrapCollisionGroup.cpp
        wrapDistanceJoint.cpp
        wrapDriveAPI.cpp
        wrapFilteredPairsAPI.cpp
        wrapFixedJoint.cpp
        wrapJoint.cpp
        wrapLimitAPI.cpp
        wrapMassAPI.cpp
        wrapMaterialAPI.cpp
        wrapMeshCollisionAPI.cpp
        wrapMetrics.cpp
        wrapPrismaticJoint.cpp
        wrapRevoluteJoint.cpp
        wrapRigidBodyAPI.cpp
        wrapScene.cpp
        wrapSphericalJoint.cpp
        wrapTokens.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/generatedSchema.usda
        resources/schema.usda:usdPhysics/schema.usda
        resources/examples/usdPhysicsBoxOnBox.usda
        resources/examples/usdPhysicsBoxOnQuad.usda
        resources/examples/usdPhysicsDistanceJoint.usda
        resources/examples/usdPhysicsGroupFiltering.usda
        resources/examples/usdPhysicsJoints.usda
        resources/examples/usdPhysicsPairFiltering.usda
        resources/examples/usdPhysicsSpheresWithMaterial.usda

    DOXYGEN_FILES
        docs/overview.dox        
)

