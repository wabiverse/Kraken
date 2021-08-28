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

  # include(InstallRequiredSystemLibraries)

  # ucrtbase(d).dll cannot be in the manifest, due to the way windows 10 handles
  # redirects for this dll, for details see T88813.
  # foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
  #   string(FIND ${lib} "ucrtbase" pos)
  #   if(NOT pos EQUAL -1)
  #     list(REMOVE_ITEM CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${lib})
  #     install(FILES ${lib} DESTINATION . COMPONENT Libraries)
  #   endif()
  # endforeach()
  # Install the CRT to the kraken.crt Sub folder.
  # install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ./kraken.manifest COMPONENT Libraries)

  # Override the default manifests from NuGet, someone
  # hardcoded everything to "MyApplication.app", how cute
  # configure_file(# MICROSOFT DEFAULT
  #   ${CMAKE_SOURCE_DIR}/release/windows/manifest/kraken.exe.manifest.in
  #   ${CMAKE_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.WinUI.1.0.0-experimental1/build/DefaultWin32Manifests/default.manifest)
  # configure_file(# MICROSOFT DEFAULT
  #   ${CMAKE_SOURCE_DIR}/release/windows/manifest/kraken.exe.manifest.in
  #   ${CMAKE_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.WinUI.1.0.0-experimental1/buildTransitive/DefaultWin32Manifests/default.manifest)
  # configure_file(# KRAKEN MANIFEST
  #   ${CMAKE_SOURCE_DIR}/release/windows/manifest/kraken.exe.manifest.in
  #   ${CMAKE_BINARY_DIR}/bin/Release/kraken.manifest)
  # configure_file(# KRAKEN MANIFEST
  #   ${CMAKE_SOURCE_DIR}/release/windows/manifest/kraken.exe.manifest.in
  #   ${CMAKE_BINARY_DIR}/source/creator/kraken.dir/Release/reunion.merged.g.manifest)
  # configure_file(# KRAKEN APPX MANIFEST
  #   ${CMAKE_SOURCE_DIR}/release/windows/appx/Package.appxmanifest
  #   ${CMAKE_BINARY_DIR}/source/creator/Package.appxmanifest)
  # configure_file(# KRAKEN APPX MANIFEST
  #   ${CMAKE_SOURCE_DIR}/release/windows/appx/Package.appxmanifest
  #   ${CMAKE_BINARY_DIR}/source/creator/kraken.dir/package.appxManifest)

  # Generating the manifest is a relativly expensive operation since
  # it is collecting an sha1 hash for every file required. so only do
  # this work when the libs have either changed or the manifest does
  # not exist yet.

  # string(SHA1 libshash "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")
  string(SHA1 libshash "ChaChaNah")
  set(manifest_trigger_file "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/crt_${libshash}")

  if(NOT EXISTS ${manifest_trigger_file})
    file(GLOB out_inst "${CMAKE_SOURCE_DIR}/release/windows/appx/assets/*.png")
    foreach(apx ${out_inst})
      get_filename_component(ff ${apx} NAME)
      file(
        INSTALL
          ${CMAKE_SOURCE_DIR}/release/windows/appx/assets/${ff}
        DESTINATION
          ${CMAKE_BINARY_DIR}/source/creator/assets
      )
      list(APPEND ASSET_FILES
        ${CMAKE_BINARY_DIR}/source/creator/assets/${ff})
    endforeach()
    # set(CRTLIBS "")
    # foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
    #   get_filename_component(filename ${lib} NAME)
    #   file(SHA1 "${lib}" sha1_file)
    #   string(APPEND CRTLIBS "    <file name=\"${filename}\" hash=\"${sha1_file}\"  hashalg=\"SHA1\" />\n")
    # endforeach()
    # configure_file(${CMAKE_SOURCE_DIR}/release/windows/manifest/kraken.exe.manifest.in ${CMAKE_CURRENT_BINARY_DIR}/app.manifest @ONLY)
    file(TOUCH ${manifest_trigger_file})
  endif()

  # install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kraken.app.manifest DESTINATION ./app.manifest)
  # set(BUNDLECRT "<dependency><dependentAssembly><assemblyIdentity name=\"Kraken.app\" version=\"1.0.0.0\" /></dependentAssembly></dependency>")
endif()

# Application Manifest & Nuget Dependencies.
set(KRAKEN_APPX_MANIFEST ${CMAKE_BINARY_DIR}/source/creator/Package.appxmanifest)
set(KRAKEN_PACKAGES_CONFIG ${CMAKE_BINARY_DIR}/packages.config)

file(GLOB out_inst_dll "${CMAKE_BINARY_DIR}/bin/Release/*.dll")
foreach(dlls ${out_inst_dll})
  get_filename_component(ffdll ${dlls} NAME)
  list(APPEND RELEASE_CONTENT_FILES ${CMAKE_BINARY_DIR}/bin/Release/${ffdll})
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

function(kraken_winrt_metadata_hotfix)
  # TODO: Fix this binary path inception weirdness.
  # I really do wonder why MSVC is looking for files
  # in kraken.dir/Release/kraken.dir/Release ...
  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/Unmerged/App.winmd)
    file(
      RENAME
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/Unmerged/App.winmd
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/kraken.dir/Release/Unmerged/App.winmd
    )
    file(
      RENAME
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/Unmerged/MainWindow.winmd
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/kraken.dir/Release/Unmerged/MainWindow.winmd
    )
    file(
      RENAME
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/Unmerged/XamlMetaDataProvider.winmd
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/kraken.dir/Release/Unmerged/XamlMetaDataProvider.winmd
    )
  endif()
  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/kraken.dir/Release/Unmerged/App.winmd)
    file(
      COPY
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/kraken.dir/Release/Unmerged/App.winmd
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/kraken.dir/Release/Unmerged/MainWindow.winmd
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/kraken.dir/Release/Unmerged/XamlMetaDataProvider.winmd
      DESTINATION
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/Unmerged
    )
  endif()
  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/Unmerged)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/Unmerged)
  endif()
  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/Merged/Kraken.winmd)
    file(
      COPY
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/Merged/Kraken.winmd
      DESTINATION
        ${CMAKE_CURRENT_BINARY_DIR}/kraken.dir/Release/kraken.dir/Release/Merged
    )
  endif()
endfunction()


# This will import NuGet packages given a relative-to-binary path
# to a project & automatically fills in the required XXX.vcxproj.user

# Just import NuGet into Kraken and Monolithic Pixar USD vcxprojs here
# As Nuget imports have already been added to wabi_library(xxx) macros
kraken_import_nuget_packages("maelstrom")

kraken_import_nuget_packages("source/creator/kraken")
kraken_import_nuget_packages("source/kraken/anchor/kraken_anchor")
kraken_import_nuget_packages("source/kraken/editors/code/kraken_editor_code")
kraken_import_nuget_packages("source/kraken/editors/screen/kraken_editor_screen")
kraken_import_nuget_packages("source/kraken/editors/space_file/kraken_editor_spacefile")
kraken_import_nuget_packages("source/kraken/editors/space_view3d/kraken_editor_spaceview3d")
kraken_import_nuget_packages("source/kraken/krakernel/kraken_kernel")
kraken_import_nuget_packages("source/kraken/kraklib/kraken_lib")
kraken_import_nuget_packages("source/kraken/luxo/kraken_luxo")
kraken_import_nuget_packages("source/kraken/python/kpy/kraken_python")
kraken_import_nuget_packages("source/kraken/server/kraken_server")
kraken_import_nuget_packages("source/kraken/universe/kraken_universe")
kraken_import_nuget_packages("source/kraken/wm/kraken_wm")

kraken_import_nuget_packages("intern/utfconv/kraken_intern_utfconv")