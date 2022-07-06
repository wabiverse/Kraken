set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usd)

wabi_library(usd
    LIBRARIES
        arch
        kind
        pcp
        sdf
        ar
        plug
        tf
        trace
        vt
        work
        ${Boost_PYTHON_LIBRARY}
        ${TBB_tbb_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        apiSchemaBase
        attribute
        clipsAPI
        attributeQuery
        collectionAPI
        collectionMembershipQuery
        common
        crateInfo
        debugCodes
        editContext
        editTarget
        errors
        flattenUtils
        inherits
        interpolation
        modelAPI
        notice
        object
        payloads
        prim
        primCompositionQuery
        primData
        primDataHandle
        primDefinition
        primFlags
        primRange
        primTypeInfo
        property
        references
        relationship
        resolveInfo
        resolver
        schemaBase
        schemaRegistry
        specializes
        stage
        stageCache
        stageCacheContext
        stageLoadRules
        stagePopulationMask
        timeCode
        tokens
        typed
        usdFileFormat
        usdaFileFormat
        usdcFileFormat
        usdzFileFormat
        variantSets
        zipFile

    PYTHON_PUBLIC_CLASSES
        pyEditContext
        pyConversions

    PRIVATE_CLASSES
        clip
        clipCache
        clipSet
        clipSetDefinition
        crateData
        crateFile
        instanceCache
        instanceKey
        integerCoding
        interpolators
        primTypeInfoCache
        usdzResolver
        valueUtils

    PUBLIC_HEADERS
        api.h

    PRIVATE_HEADERS
        crateDataTypes.h
        crateValueInliners.h
        listEditImpl.h
        wrapUtils.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapAPISchemaBase.cpp
        wrapAttribute.cpp
        wrapAttributeQuery.cpp
        wrapClipsAPI.cpp
        wrapCollectionAPI.cpp
        wrapCollectionMembershipQuery.cpp
        wrapCommon.cpp
        wrapCrateInfo.cpp
        wrapEditContext.cpp
        wrapEditTarget.cpp
        wrapFlattenUtils.cpp
        wrapInherits.cpp
        wrapInterpolation.cpp
        wrapModelAPI.cpp
        wrapNotice.cpp
        wrapObject.cpp
        wrapPayloads.cpp
        wrapPrim.cpp
        wrapPrimCompositionQuery.cpp
        wrapPrimDefinition.cpp
        wrapPrimFlags.cpp
        wrapPrimRange.cpp
        wrapPrimTypeInfo.cpp
        wrapProperty.cpp
        wrapReferences.cpp
        wrapRelationship.cpp
        wrapResolveInfo.cpp
        wrapSchemaBase.cpp
        wrapSchemaRegistry.cpp
        wrapSpecializes.cpp
        wrapStage.cpp
        wrapStageCache.cpp
        wrapStageCacheContext.cpp
        wrapStageLoadRules.cpp
        wrapStagePopulationMask.cpp
        wrapTokens.cpp
        wrapTimeCode.cpp
        wrapTyped.cpp
        wrapUtils.cpp
        wrapVariantSets.cpp
        wrapVersion.cpp
        wrapZipFile.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json
        resources/generatedSchema.usda
        resources/schema.usda:usd/schema.usda
        resources/codegenTemplates/api.h
        resources/codegenTemplates/plugInfo.json:no_subst
        resources/codegenTemplates/schemaClass.cpp
        resources/codegenTemplates/schemaClass.h
        resources/codegenTemplates/tokens.cpp
        resources/codegenTemplates/tokens.h
        resources/codegenTemplates/wrapSchemaClass.cpp
        resources/codegenTemplates/wrapTokens.cpp

    DOXYGEN_FILES
        docs/advancedFeatures.dox
        docs/authoringEditing.dox
        docs/bestPractices.dox
        docs/commonIdioms.dox
        docs/datatypes.dox
        docs/dynamicFileFormat.dox
        docs/front.dox
        docs/generatingSchemas.dox
        docs/instancing.dox
        docs/multiThreading.dox
        docs/objectModel.dox
        docs/propertiesOfSceneDescription.dox 
        docs/valueClips.dox
        docs/images/instancing/Instancing_Example.png
        docs/images/instancing/Nested_Instancing_Example.png
        docs/images/instancing/Relationship_Example.png
        docs/images/instancing/Uninstanced_vs_Instanced.png
)


if(NOT WITH_PYTHON)
    message(STATUS "Skipping building usdGenSchema, Python modules required.")
elseif(NOT JINJA2_FOUND)
    message(WARNING "Skipping building usdGenSchema due to missing dependency: Jinja2")
else()
    # wabi_python_bin(usdGenSchema
    #     DEPENDENCIES
    #         tf
    #         sdf
    #         usd
    # )
endif()


