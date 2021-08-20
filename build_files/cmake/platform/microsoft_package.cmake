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
    unset(MSVC_REDIST_DIR CACHE)
  endif()

  include(InstallRequiredSystemLibraries)

  # ucrtbase(d).dll cannot be in the manifest, due to the way windows 10 handles
  # redirects for this dll, for details see T88813.
  foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
    string(FIND ${lib} "ucrtbase" pos)
    if(NOT pos EQUAL -1)
      list(REMOVE_ITEM CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${lib})
      install(FILES ${lib} DESTINATION . COMPONENT Libraries)
    endif()
  endforeach()
  # Install the CRT to the kraken.crt Sub folder.
  install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ./kraken.crt COMPONENT Libraries)

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
      string(APPEND CRTLIBS "    <file name=\"${filename}\" hash=\"${sha1_file}\"  hashalg=\"SHA1\" />\n")
    endforeach()
    configure_file(${CMAKE_SOURCE_DIR}/release/windows/manifest/kraken.crt.manifest.in ${CMAKE_CURRENT_BINARY_DIR}/kraken.crt.manifest @ONLY)
    file(TOUCH ${manifest_trigger_file})
  endif()

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kraken.crt.manifest DESTINATION ./kraken.crt)
  set(BUNDLECRT "<dependency><dependentAssembly><assemblyIdentity name=\"WabiAnimation.app\" version=\"1.0.0.0\" /></dependentAssembly></dependency>")
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/release/windows/appx/Package.appxmanifest
  ${CMAKE_BINARY_DIR}/source/creator/Package.appxmanifest
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/release/windows/packages.config
  ${CMAKE_BINARY_DIR}/source/creator/packages.config
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/microsoft/App.xaml
  ${CMAKE_BINARY_DIR}/source/creator/App.xaml
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/microsoft/MainWindow.xaml
  ${CMAKE_BINARY_DIR}/source/creator/MainWindow.xaml
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/microsoft/App.idl
  ${CMAKE_BINARY_DIR}/source/creator/App.idl
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/microsoft/MainWindow.idl
  ${CMAKE_BINARY_DIR}/source/creator/MainWindow.idl
  @ONLY)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/microsoft/pch.h
  ${CMAKE_BINARY_DIR}/source/creator/pch.h
  @ONLY
)

configure_file(
  ${CMAKE_SOURCE_DIR}/source/creator/microsoft/pch.cpp
  ${CMAKE_BINARY_DIR}/source/creator/pch.cpp
  @ONLY
)

# Resource Paths Assets.
set(KRAKEN_RESOURCE_RC ${CMAKE_SOURCE_DIR}/release/windows/icons/winkraken.rc)
list(APPEND STRING_FILES
  ${KRAKEN_RESOURCE_RC}
)

# Application Manifest & Nuget Dependencies.
set(KRAKEN_APPX_MANIFEST ${CMAKE_BINARY_DIR}/source/creator/Package.appxmanifest)
set(KRAKEN_PACKAGES_CONFIG ${CMAKE_BINARY_DIR}/source/creator/packages.config)

# XAML to define WinRT runtime typing & MIDL for language projection.
set(KRAKEN_APPLICATION_DEFINITION_XAML ${CMAKE_BINARY_DIR}/source/creator/App.xaml)
set(KRAKEN_APPLICATION_DEFINITION_MIDL ${CMAKE_BINARY_DIR}/source/creator/App.idl)
set(KRAKEN_MAIN_PAGE_XAML ${CMAKE_BINARY_DIR}/source/creator/MainWindow.xaml)
set(KRAKEN_MAIN_PAGE_MIDL ${CMAKE_BINARY_DIR}/source/creator/MainWindow.idl)

list(APPEND CONTENT_FILES
  ${KRAKEN_APPX_MANIFEST}
  ${KRAKEN_PACKAGES_CONFIG}
  ${KRAKEN_APPLICATION_DEFINITION_XAML}
  ${KRAKEN_APPLICATION_DEFINITION_MIDL}
  ${KRAKEN_MAIN_PAGE_XAML}
  ${KRAKEN_MAIN_PAGE_MIDL}
)

set(USER_PROPS_FILE "${CMAKE_CURRENT_BINARY_DIR}/source/creator/kraken.vcxproj.user")


file(TO_CMAKE_PATH
  "${CMAKE_CURRENT_BINARY_DIR}/packages/Microsoft.Windows.CppWinRT.2.0.210806.1/build/native/Microsoft.Windows.CppWinRT.props"
  MICROSOFT_CPP_WINRT_PROJECT_PROPS
)
file(TO_CMAKE_PATH
  "${CMAKE_CURRENT_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.DWrite.1.0.0-experimental1/build/Microsoft.WindowsAppSDK.DWrite.props"
  MICROSOFT_APP_SDK_DWRITE_PROPS
)
file(TO_CMAKE_PATH
  "${CMAKE_CURRENT_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.InteractiveExperiences.1.0.0-experimental1/build/native/Microsoft.WindowsAppSDK.InteractiveExperiences.props"
  MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES_PROPS
)
file(TO_CMAKE_PATH
  "${CMAKE_CURRENT_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.WinUI.1.0.0-experimental1/build/native/Microsoft.WindowsAppSDK.WinUI.props"
  MICROSOFT_APP_SDK_WINUI_PROPS
)
file(TO_CMAKE_PATH
  "${CMAKE_CURRENT_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.Foundation.1.0.0-experimental1/build/native/Microsoft.WindowsAppSDK.Foundation.props"
  MICROSOFT_APP_SDK_FOUNDATION_PROPS
)
file(TO_CMAKE_PATH
  "${CMAKE_CURRENT_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.1.0.0-experimental1/build/native/Microsoft.WindowsAppSDK.props"
  MICROSOFT_APP_SDK_EXPERIMENTAL_PROPS
)

# if(NOT EXISTS ${USER_PROPS_FILE})
  # Layout below is messy, because otherwise the generated file will look messy.
file(WRITE ${USER_PROPS_FILE} "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Project xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
  <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT_PROPS}\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT_PROPS}\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_DWRITE_PROPS}\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_DWRITE_PROPS}\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES_PROPS}\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES_PROPS}\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_WINUI_PROPS}\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_WINUI_PROPS}\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_FOUNDATION_PROPS}\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_FOUNDATION_PROPS}\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_EXPERIMENTAL_PROPS}\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_EXPERIMENTAL_PROPS}\')\" />
</Project>")
# endif()

file(GLOB out_inst_dll "${CMAKE_BINARY_DIR}/bin/Release/*.dll")
foreach(dlls ${out_inst_dll})
  get_filename_component(ffdll ${dlls} NAME)
  list(APPEND RELEASE_CONTENT_FILES ${CMAKE_BINARY_DIR}/bin/Release/${ffdll})
endforeach()

file(GLOB out_inst_winmd "${CMAKE_BINARY_DIR}/bin/Release/*.winmd")
foreach(winmds ${out_inst_winmd})
  get_filename_component(ffwinmd ${winmds} NAME)
  list(APPEND ASSET_FILES ${CMAKE_BINARY_DIR}/bin/Release/${ffwinmd})
endforeach()

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

set_property(SOURCE
  ${CONTENT_FILES}
  PROPERTY VS_DEPLOYMENT_CONTENT 1)

set_property(SOURCE
  ${ASSET_FILES}
  PROPERTY VS_DEPLOYMENT_CONTENT 1)

set_property(SOURCE
  ${STRING_FILES}
  PROPERTY VS_TOOL_OVERRIDE "PRIResource")

set_property(SOURCE
  ${ASSET_FILES}
  PROPERTY VS_DEPLOYMENT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/source/creator")

set_property(SOURCE
  ${CONTENT_FILES}
  PROPERTY VS_DEPLOYMENT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/source/creator")

if(KRAKEN_RELEASE_MODE)
  set_property(SOURCE ${RELEASE_CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
else()
  set_property(SOURCE ${DEBUG_CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
endif()

add_custom_target(appximages ALL SOURCES ${ASSET_FILES})
add_custom_target(appxml ALL SOURCES ${CONTENT_FILES})

add_dependencies(appxml appximages)

# set_target_properties(appxml PROPERTIES
#   VS_PACKAGE_REFERENCES "Microsoft.Windows.CppWinRT_2.0.210806.1;Microsoft.WindowsAppSDK.DWrite_1.0.0-experimental1;Microsoft.WindowsAppSDK_1.0.0-experimental1;Microsoft.WindowsAppSDK.Foundation_1.0.0-experimental1;Microsoft.WindowsAppSDK.WinUI_1.0.0-experimental1"
#   VS_GLOBAL_UseWindowsSdkPreview "true"
#   VS_GLOBAL_WindowsSdkPackageVersion "10.0.22000.160-preview"
#   VS_GLOBAL_RootNamespace "Kraken"
#   VS_GLOBAL_ProjectName "Kraken"
#   VS_GLOBAL_UseWinUI "true"
#   VS_GLOBAL_CanReferenceWinRT "true"
#   VS_GLOBAL_XamlLanguage "CppWinRT"
# )