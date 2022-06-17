set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdAbc)

if(NOT ALEMBIC_FOUND)
    message(WARNING "Not building ${WABI_PACKAGE} because of missing dependency: Alembic")
    return()
endif()

set(optionalLibs "")
set(optionalIncludeDirs "")

if (WITH_ALEMBIC_HDF5)
   add_definitions(-DWITH_ALEMBIC_HDF5)
   list(APPEND optionalLibs ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})
   list(APPEND optionalIncludeDirs ${HDF5_INCLUDE_DIRS})
endif()

wabi_plugin(usdAbc
    LIBRARIES
        tf
        work
        sdf
        usd
        usdGeom
        ${ALEMBIC_LIBRARIES}
        ${OPENEXR_Half_LIBRARY} 
        ${OPENEXR_Imath_LIBRARY} 
        ${OPENEXR_Iex_LIBRARY} 
        ${OPENEXR_IexMath_LIBRARY}
        ${OPENEXR_LIBRARIES}
        ${IMATH_LIBRARY}
        ${optionalLibs}

    INCLUDE_DIRS
        ${ALEMBIC_INCLUDE_DIR}
        ${ILMBASE_INCLUDES}/Imath
        ${OPENEXR_INCLUDE_DIR}
        ${OPENEXR_INCLUDE_DIRS}
        ${optionalIncludeDirs}

    PRIVATE_CLASSES
        alembicData
        alembicFileFormat
        alembicReader
        alembicTest
        alembicUtil
        alembicWriter

    PRIVATE_HEADERS
        api.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapAlembicTest.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)
