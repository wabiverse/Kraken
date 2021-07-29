# First generate the manifest for tests since it will not need the dependency on the CRT.
configure_file(
  ${CMAKE_SOURCE_DIR}/release/windows/appx/Package.appxmanifest
  ${CMAKE_CURRENT_BINARY_DIR}/Tests.appxmanifest @ONLY
  @ONLY)

set(CONTENT_FILES "")
set(ASSET_FILES "")
set(STRING_FILES "")
set(DEBUG_CONTENT_FILES "")
set(RELEASE_CONTENT_FILES "")

configure_file(
  ${CMAKE_SOURCE_DIR}/release/windows/appx/priconfig.xml.in
  ${CMAKE_CURRENT_BINARY_DIR}/tests.priconfig.xml
  @ONLY)

if(WITH_WINDOWS_BUNDLE_CRT)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)

  # This sometimes can change when updates are installed and the compiler version
  # changes, so test if it exists and if not, give InstallRequiredSystemLibraries
  # another chance to figure out the path.
  if(MSVC_REDIST_DIR AND NOT EXISTS "${MSVC_REDIST_DIR}")
    # Align this with Windows 11 MSVC 2022 Runtime
    list(APPEND MSVC_REDIST_DIR
      "C:/Program Files (x86)/Microsoft Visual Studio/2022/Preview/VC/Redist/MSVC/14.30.30401")
  endif()

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
  set(BUNDLECRT "<dependency><dependentAssembly><assemblyIdentity name=\"Kraken.app\" version=\"1.0.0.0\" /></dependentAssembly></dependency>")
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/release/windows/appx/priconfig.xml.in
  ${CMAKE_CURRENT_BINARY_DIR}/priconfig.xml
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/release/windows/appx/Package.appxmanifest
  ${CMAKE_CURRENT_BINARY_DIR}/Package.appxmanifest
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/release/windows/appx/App.xml
  ${CMAKE_CURRENT_BINARY_DIR}/App.xml
  @ONLY)

# Resource Paths Assets.
set(KRAKEN_PRI_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/priconfig.xml)
set(KRAKEN_RESOURCE_RC ${CMAKE_SOURCE_DIR}/release/windows/icons/winkraken.rc)
list(APPEND STRING_FILES
  ${KRAKEN_PRI_CONFIG}
  ${KRAKEN_RESOURCE_RC}
)

# Manifest Assets.
set(KRAKEN_APPX_MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/Package.appxmanifest)
set(KRAKEN_APPX_XML ${CMAKE_CURRENT_BINARY_DIR}/App.xml)
list(APPEND CONTENT_FILES
  ${KRAKEN_APPX_MANIFEST}
  ${KRAKEN_APPX_XML}
)

# Icon Assets.
set(KRAKEN_APPX_ICONS ${CMAKE_SOURCE_DIR}/release/windows/appx/assets)
set(KRAKEN_ALL_ICONS ${CMAKE_SOURCE_DIR}/release/windows/icons)
list(APPEND ASSET_FILES
  ${KRAKEN_APPX_ICONS}
  ${KRAKEN_ALL_ICONS}
)

set_property(SOURCE PROPERTY ${CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE PROPERTY ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE PROPERTY ${STRING_FILES} PROPERTY VS_TOOL_OVERRIDE "PRIResource")
set_property(SOURCE PROPERTY ${DEBUG_CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT $<CONFIG:Debug>)
set_property(SOURCE PROPERTY ${RELEASE_CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT $<CONFIG:Release,RelWithDebInfo,MinSizeRel>)

file(GLOB out_inst "${CMAKE_SOURCE_DIR}/release/windows/appx/assets/*.png")
foreach(apx ${out_inst})
  get_filename_component(ff ${apx} NAME)
  # configure_file(
  #   ${CMAKE_SOURCE_DIR}/release/windows/appx/assets/${ff}
  #   ${CMAKE_BINARY_DIR}/source/creator/kraken.dir/Release/PackageLayout/assets/${ff}
  #   @ONLY)
  file(
    INSTALL
      ${CMAKE_SOURCE_DIR}/release/windows/appx/assets/${ff}
    DESTINATION
      ${CMAKE_CURRENT_BINARY_DIR}/bin/release/AppX/assets
  )
  list(APPEND ASSET_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/bin/release/AppX/assets/${ff})
endforeach()

add_custom_target(appximages ALL SOURCES ${ASSET_FILES})
add_custom_target(appxml ALL SOURCES ${CONTENT_FILES})

add_dependencies(appxml appximages)

set_property(SOURCE PROPERTY ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/bin/release/AppX")
