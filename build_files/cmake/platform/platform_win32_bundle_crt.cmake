# First generate the manifest for tests since it will not need the dependency on the CRT.
set(CONTENT_FILES "")
set(ASSET_FILES "")
set(STRING_FILES "")
set(DEBUG_CONTENT_FILES "")
set(RELEASE_CONTENT_FILES "")

set(MSVC_TOOLSET_VERSION 143)

if(WITH_WINDOWS_BUNDLE_CRT)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)

  # This sometimes can change when updates are installed and the compiler version
  # changes, so test if it exists and if not, give InstallRequiredSystemLibraries
  # another chance to figure out the path.
  if(MSVC_REDIST_DIR AND NOT EXISTS "${MSVC_REDIST_DIR}")
    file(TO_CMAKE_PATH
      "C:/Program Files/Microsoft Visual Studio/2022/Preview/VC/Redist/MSVC/14.30.30401"
      MSVC_REDIST_DIR)
  endif()
    # Align this with Windows 11 MSVC 2022 Runtime


    file(TO_CMAKE_PATH
      "C:/Program Files/Microsoft Visual Studio/2022/Preview/VC/Redist/MSVC/14.30.30401/x64/Microsoft.VC142.CRT"
      WINDOWS_11_MSVC_REDIST
    )
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
      ${WINDOWS_11_MSVC_REDIST}/concrt140_app.dll
      ${WINDOWS_11_MSVC_REDIST}/concrt140.dll
      ${WINDOWS_11_MSVC_REDIST}/msvcp140_1_app.dll
      ${WINDOWS_11_MSVC_REDIST}/msvcp140_1.dll
      ${WINDOWS_11_MSVC_REDIST}/msvcp140_2_app.dll
      ${WINDOWS_11_MSVC_REDIST}/msvcp140_2.dll
      ${WINDOWS_11_MSVC_REDIST}/msvcp140_app.dll
      ${WINDOWS_11_MSVC_REDIST}/msvcp140_atomic_wait_app.dll
      ${WINDOWS_11_MSVC_REDIST}/msvcp140_atomic_wait.dll
      ${WINDOWS_11_MSVC_REDIST}/msvcp140_codecvt_ids_app.dll
      ${WINDOWS_11_MSVC_REDIST}/msvcp140_codecvt_ids.dll
      ${WINDOWS_11_MSVC_REDIST}/msvcp140.dll
      ${WINDOWS_11_MSVC_REDIST}/vcamp140_app.dll
      ${WINDOWS_11_MSVC_REDIST}/vccorlib140_app.dll
      ${WINDOWS_11_MSVC_REDIST}/vccorlib140.dll
      ${WINDOWS_11_MSVC_REDIST}/vcomp140_app.dll
      ${WINDOWS_11_MSVC_REDIST}/vcruntime140_1_app.dll
      ${WINDOWS_11_MSVC_REDIST}/vcruntime140_1.dll
      ${WINDOWS_11_MSVC_REDIST}/vcruntime140_app.dll
      ${WINDOWS_11_MSVC_REDIST}/vcruntime140.dll
    )

  include(InstallRequiredSystemLibraries)

  # Install the CRT to the kraken.crt Sub folder.
  install(
    FILES
      ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION
      ./kraken.crt COMPONENT Libraries)

  # Generating the manifest is a relativly expensive operation since
  # it is collecting an sha1 hash for every file required. so only do
  # this work when the libs have either changed or the manifest does
  # not exist yet.

  string(SHA1 libshash "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")
  set(manifest_trigger_file "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/crt_${libshash}")

  if(NOT EXISTS ${manifest_trigger_file})
    file(GLOB out_inst "${CMAKE_SOURCE_DIR}/release/windows/appx/assets/*.png")
    foreach(apx ${out_inst})
      get_filename_component(ff ${apx} NAME)
      file(
        INSTALL
          ${CMAKE_SOURCE_DIR}/release/windows/appx/assets/${ff}
        DESTINATION
          ${CMAKE_CURRENT_BINARY_DIR}/source/creator/assets
      )
      list(APPEND ASSET_FILES
        ${CMAKE_BINARY_DIR}/source/creator/assets/${ff})
    endforeach()
    set(CRTLIBS "")
    foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
      get_filename_component(filename ${lib} NAME)
      file(SHA1 "${lib}" sha1_file)
      set(CRTLIBS "${CRTLIBS}    <file name=\"${filename}\" hash=\"${sha1_file}\"  hashalg=\"SHA1\" />\n")
    endforeach()
    configure_file(${CMAKE_SOURCE_DIR}/release/windows/manifest/kraken.crt.manifest.in ${CMAKE_CURRENT_BINARY_DIR}/kraken.crt.manifest @ONLY)
    file(TOUCH ${manifest_trigger_file})
  endif()

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kraken.crt.manifest DESTINATION ./kraken.crt)
  set(BUNDLECRT "<dependency><dependentAssembly><assemblyIdentity name=\"Kraken.CRT\" version=\"1.0.0.0\" /></dependentAssembly></dependency>")
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/release/windows/appx/priconfig.xml.in
  ${CMAKE_BINARY_DIR}/source/creator/priconfig.xml
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/release/windows/appx/Package.appxmanifest
  ${CMAKE_BINARY_DIR}/source/creator/Package.appxmanifest
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/release/windows/packages.config
  ${CMAKE_BINARY_DIR}/source/creator/packages.config
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/Creator.xaml
  ${CMAKE_BINARY_DIR}/source/creator/Creator.xaml
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/Creator.idl
  ${CMAKE_BINARY_DIR}/source/creator/Creator.idl
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/creator_xaml_typeinfo.h
  ${CMAKE_BINARY_DIR}/source/creator/creator_xaml_typeinfo.h
  @ONLY
)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/creator_xaml_metadata.h
  ${CMAKE_BINARY_DIR}/source/creator/creator_xaml_metadata.h
  @ONLY
)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/creator_xaml_metadata.cpp
  ${CMAKE_BINARY_DIR}/source/creator/creator_xaml_metadata.cpp
  @ONLY
)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/creator_xaml_typeinfo.cpp
  ${CMAKE_BINARY_DIR}/source/creator/creator_xaml_typeinfo.cpp
  @ONLY
)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/pch.h
  ${CMAKE_BINARY_DIR}/source/creator/pch.h
  @ONLY
)

# Resource Paths Assets.
set(KRAKEN_PRI_CONFIG ${CMAKE_BINARY_DIR}/source/creator/priconfig.xml)
set(KRAKEN_RESOURCE_RC ${CMAKE_SOURCE_DIR}/release/windows/icons/winkraken.rc)
list(APPEND STRING_FILES
  ${KRAKEN_PRI_CONFIG}
  ${KRAKEN_RESOURCE_RC}
)

# Manifest Assets.
set(KRAKEN_METADATA_SOURCE ${CMAKE_BINARY_DIR}/source/creator/creator_xaml_metadata.cpp)
set(KRAKEN_TYPEINFO_SOURCE ${CMAKE_BINARY_DIR}/source/creator/creator_xaml_typeinfo.cpp)
set(KRAKEN_TYPEINFO_HEADER ${CMAKE_BINARY_DIR}/source/creator/creator_xaml_typeinfo.h)
set(KRAKEN_METADATA_HEADER ${CMAKE_BINARY_DIR}/source/creator/creator_xaml_metadata.h)


list(APPEND KRAKEN_CREATOR_SOURCE_FILES
  ${KRAKEN_METADATA_SOURCE}
  ${KRAKEN_TYPEINFO_SOURCE}
  ${KRAKEN_TYPEINFO_HEADER}
  ${KRAKEN_METADATA_HEADER}
)

set(KRAKEN_APPX_MANIFEST ${CMAKE_BINARY_DIR}/source/creator/Package.appxmanifest)
set(KRAKEN_PACKAGES_CONFIG ${CMAKE_BINARY_DIR}/source/creator/packages.config)
set(KRAKEN_APPX_XML ${CMAKE_BINARY_DIR}/source/creator/Creator.xaml)
set(KRAKEN_APPX_IDL ${CMAKE_BINARY_DIR}/source/creator/Creator.idl)
list(APPEND CONTENT_FILES
  ${KRAKEN_APPX_MANIFEST}
  ${KRAKEN_PACKAGES_CONFIG}
  ${KRAKEN_APPX_XML}
  ${KRAKEN_APPX_IDL}
)

file(GLOB out_inst_ico "${CMAKE_SOURCE_DIR}/release/windows/icons/*.png")
foreach(ico ${out_inst_ico})
  get_filename_component(ffico ${ico} NAME)
  list(APPEND ASSET_FILES ${CMAKE_BINARY_DIR}/release/windows/icons/${ffico})
endforeach()

file(GLOB out_inst "${CMAKE_SOURCE_DIR}/release/windows/appx/assets/*.png")
foreach(apx ${out_inst})
  get_filename_component(ff ${apx} NAME)
  list(APPEND ASSET_FILES ${CMAKE_BINARY_DIR}/source/creator/assets/${ff})
endforeach()

set_property(SOURCE ${CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${STRING_FILES} PROPERTY VS_TOOL_OVERRIDE "PRIResource")
set_property(SOURCE ${DEBUG_CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT $<CONFIG:Debug>)
set_property(SOURCE ${RELEASE_CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT $<CONFIG:Release,RelWithDebInfo,MinSizeRel>)

add_custom_target(appximages ALL SOURCES ${ASSET_FILES})
add_custom_target(appxml ALL SOURCES ${CONTENT_FILES})

add_dependencies(appxml appximages)

set_property(SOURCE PROPERTY ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_LOCATION "${CMAKE_BINARY_DIR}/source/creator")
