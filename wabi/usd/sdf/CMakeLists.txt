set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE sdf)

wabi_library(sdf
    LIBRARIES
        arch
        tf
        gf
        trace
        vt
        work
        ar
        ${Boost_PYTHON_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}

    PUBLIC_CLASSES
        abstractData
        allowed
        assetPath
        attributeSpec
        changeBlock
        changeList
        children
        childrenPolicies
        childrenProxy
        childrenUtils
        childrenView
        cleanupEnabler
        copyUtils
        data
        declareHandles
        fileFormat
        identity
        layer
        layerOffset
        layerStateDelegate
        layerTree
        layerUtils
        listProxy
        listEditor
        listEditorProxy
        listOp
        mapEditProxy
        mapEditor
        namespaceEdit
        notice
        path
        pathNode
        pathTable
        payload
        pool
        primSpec
        propertySpec
        proxyPolicies
        proxyTypes
        pseudoRootSpec
        reference
        relationshipSpec
        schema
        site
        siteUtils
        spec
        specType
        textFileFormat
        timeCode
        tokens
        types
        valueTypeName
        variantSetSpec
        variantSpec

        # Flex/Bison generated code
        path.tab
        textFileFormat.tab
   
    PYTHON_PUBLIC_CLASSES
        pyChildrenProxy
        pyChildrenView
        pyListEditorProxy
        pyListOp
        pyListProxy
        pyMapEditProxy
        pySpec
        pyUtils

    PUBLIC_HEADERS
        api.h
        accessorHelpers.h
        declareSpec.h
        layerHints.h
        schemaTypeRegistration.h

    PRIVATE_CLASSES
        assetPathResolver
        changeManager
        cleanupTracker
        connectionListEditor
        debugCodes
        fileFormatRegistry
        fileIO
        fileIO_Common
        layerRegistry
        listOpListEditor
        parserHelpers
        parserValueContext
        pathParser
        subLayerListEditor
        textParserContext
        valueTypeRegistry
        vectorListEditor

    PRIVATE_HEADERS
        instantiatePool.h
        valueTypePrivate.h

    CPPFILES
        # Flex/Bison generated code
        path.lex.cpp
        textFileFormat.lex.cpp

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapArrayAssetPath.cpp
        wrapArrayPath.cpp
        wrapArrayTimeCode.cpp
        wrapAssetPath.cpp
        wrapAttributeSpec.cpp
        wrapChangeBlock.cpp
        wrapCleanupEnabler.cpp
        wrapCopyUtils.cpp
        wrapFileFormat.cpp
        wrapLayer.cpp
        wrapLayerOffset.cpp
        wrapLayerTree.cpp
        wrapNamespaceEdit.cpp
        wrapNotice.cpp
        wrapPath.cpp
        wrapPayload.cpp
        wrapPrimSpec.cpp
        wrapPropertySpec.cpp
        wrapPseudoRootSpec.cpp
        wrapRelationshipSpec.cpp
        wrapReference.cpp
        wrapSpec.cpp
        wrapTimeCode.cpp
        wrapTypes.cpp
        wrapValueTypeName.cpp
        wrapVariantSpec.cpp
        wrapVariantSetSpec.cpp

    PYMODULE_FILES 
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)
