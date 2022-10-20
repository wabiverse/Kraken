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

  string(SHA1 libshash "ChaChaNah")
  set(manifest_trigger_file "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/crt_${libshash}")
endif()

# Generates all the icons necessary to package the Kraken Application for the
# Microsoft App Store deployment, as well as nicely packaging our application
# for local deployments. Command 'GenerateKrakenAssets' is defined within the
# KrakenDeveloperProfile.ps1 pwsh profile.
macro(gen_app_icons)
  execute_process(
    COMMAND pwsh -ExecutionPolicy Unrestricted -Command "GenerateKrakenAssets -Path ${CMAKE_SOURCE_DIR}/release/windows/appx/assets/kraken.svg -Destination ./release/windows/appx/assets"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE icongen_success)
    if(icongen_success EQUAL "1")
      message(FATAL_ERROR "Icons assets could not be generated.")
    else()
      file(GLOB GENERATED_APP_ICONS "${CMAKE_BINARY_DIR}/release/windows/appx/assets/*.png")
      foreach(app_icon_file ${GENERATED_APP_ICONS})
        get_filename_component(icon_name ${app_icon_file} NAME)
        list(APPEND ASSET_FILES ${CMAKE_BINARY_DIR}/release/windows/appx/assets/${icon_name})
      endforeach()
    endif()
endmacro()

# Application Manifest & Nuget Dependencies.
set(KRAKEN_APPX_MANIFEST ${CMAKE_BINARY_DIR}/source/creator/Package.appxmanifest)
set(KRAKEN_PACKAGES_CONFIG ${CMAKE_BINARY_DIR}/packages.config)
set(KRAKEN_NUGET_CONFIG ${CMAKE_BINARY_DIR}/NuGet.config)

file(GLOB out_inst_dll "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/*.dll")
foreach(dlls ${out_inst_dll})
  get_filename_component(ffdll ${dlls} NAME)
  list(APPEND RELEASE_CONTENT_FILES ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${ffdll})
endforeach()

file(GLOB out_inst_ico "${CMAKE_SOURCE_DIR}/release/windows/icons/*.png")
foreach(ico ${out_inst_ico})
  get_filename_component(ffico ${ico} NAME)
  list(APPEND ASSET_FILES ${CMAKE_BINARY_DIR}/release/windows/icons/${ffico})
endforeach()

function(kraken_chaosengine_metadata_hotfix)
  # TODO: Fix this binary path inception weirdness.
  # I really do wonder why MSVC is looking for files
  # in kraken.dir/${CMAKE_BUILD_TYPE}/kraken.dir/${CMAKE_BUILD_TYPE} ...
  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/${CMAKE_BUILD_TYPE}/Unmerged/App.winmd)
    file(
      COPY
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/${CMAKE_BUILD_TYPE}/Unmerged/App.winmd
        # ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/${CMAKE_BUILD_TYPE}/Unmerged/MainWindow.winmd
        # ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/${CMAKE_BUILD_TYPE}/Unmerged/MainPage.winmd
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/${CMAKE_BUILD_TYPE}/Unmerged/XamlMetaDataProvider.winmd
      DESTINATION
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/${CMAKE_BUILD_TYPE}/kraken.dir/${CMAKE_BUILD_TYPE}/Unmerged
    )
  endif()
  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/${CMAKE_BUILD_TYPE}/Unmerged)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/${CMAKE_BUILD_TYPE}/Unmerged)
  endif()
  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/${CMAKE_BUILD_TYPE}/Merged/Kraken.winmd)
    file(
      COPY
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/${CMAKE_BUILD_TYPE}/Merged/Kraken.winmd
      DESTINATION
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/${CMAKE_BUILD_TYPE}/kraken.dir/${CMAKE_BUILD_TYPE}/Merged
    )
  endif()
endfunction()


# This will import NuGet packages given a relative-to-binary path
# to a project & automatically fills in the required XXX.vcxproj.user

# Just import NuGet into Kraken and Monolithic Pixar USD vcxprojs here
# As Nuget imports have already been added to wabi_library(xxx) macros
kraken_import_nuget_packages("maelstrom")

kraken_import_nuget_packages("source/creator/buildinfo")
kraken_import_nuget_packages("source/creator/kraken")
kraken_import_nuget_packages("source/kraken/anchor/kraken_anchor")
kraken_import_nuget_packages("source/kraken/editors/code/kraken_editor_code")
kraken_import_nuget_packages("source/kraken/editors/screen/kraken_editor_screen")
kraken_import_nuget_packages("source/kraken/editors/space_file/kraken_editor_spacefile")
kraken_import_nuget_packages("source/kraken/editors/space_view3d/kraken_editor_space_view3d")
kraken_import_nuget_packages("source/kraken/krakernel/kraken_kernel")
kraken_import_nuget_packages("source/kraken/kraklib/kraken_lib")
kraken_import_nuget_packages("source/kraken/luxo/kraken_luxo")
kraken_import_nuget_packages("source/kraken/python/intern/kraken_python")
kraken_import_nuget_packages("source/kraken/server/kraken_server")
kraken_import_nuget_packages("source/kraken/universe/kraken_universe")
kraken_import_nuget_packages("source/kraken/wm/kraken_wm")

kraken_import_nuget_packages("intern/utfconv/kraken_intern_utfconv")