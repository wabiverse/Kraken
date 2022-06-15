set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usdDraco)

if(WIN32 AND NOT CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
  set(USD_DRACO_SET_WINDOWS_EXPORT 1)
endif()

wabi_plugin(${WABI_PACKAGE}
    LIBRARIES
        tf
        gf
        sdf
        usd
        usdGeom
        ${DRACO_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        ${DRACO_INCLUDES}
        ${PYTHON_INCLUDE_DIRS}

    CPPFILES
        attributeDescriptor.cpp
        attributeFactory.cpp
        exportTranslator.cpp
        fileFormat.cpp
        importTranslator.cpp
        writer.cpp

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapDraco.cpp

    PYMODULE_FILES
        python/__init__.py

    RESOURCE_FILES
        resources/plugInfo.json

    DOXYGEN_FILES
        docs/overview.dox
)

if(USD_DRACO_SET_WINDOWS_EXPORT)
  unset(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS)
endif()

