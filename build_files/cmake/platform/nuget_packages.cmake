# This will work as a NuGet package manager of sorts,
# providing the dependencies C++/WinRT, the Microsoft
# Windows 11 SDK, Experimental Windows Features as
# well as the WinUI 3 package. This is necessary in
# order to consume these modern new APIs without the
# need for Visual Studio IDE, and continue natively
# building Kraken from within CMake.

# Determines the version of a native nuget package from the root packages.config.
#
# id  : package id
# out : name of variable to set result
function(package_version id out packages_config)
    file(READ ${packages_config} packages_config_contents)
    string(REGEX MATCH "package[ ]*id[ ]*=[ ]*\"${id}\"" found_package_id ${packages_config_contents})
    if (NOT(found_package_id))
        message(FATAL_ERROR "Could not find '${id}' in packages.config!")
    endif()

    set(pattern ".*id[ ]*=[ ]*\"${id}\"[ ]+version=\"([0-9a-zA-Z\\.-]+)\"[ ]+targetFramework.*")
    string(REGEX REPLACE ${pattern} "\\1" version ${packages_config_contents})
    set(${out} ${version} PARENT_SCOPE)
endfunction()

# Downloads the nuget packages based on packages.config 
function(
    add_fetch_nuget_target
    nuget_target # Target to be written to
    target_dependency # The file in the nuget package that is needed
)
    # Pull down the nuget packages
    if (NOT(MSVC) OR NOT(WIN32))
      message(FATAL_ERROR "NuGet packages are only supported for MSVC on Windows.")
    endif()

    # Retrieve the latest version of nuget
    include(ExternalProject)
    ExternalProject_Add(nuget_exe
    PREFIX nuget_exe
    URL "https://dist.nuget.org/win-x86-commandline/v6.3.0/nuget.exe"
    DOWNLOAD_NO_EXTRACT 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    UPDATE_COMMAND ""
    INSTALL_COMMAND "")

    set(NUGET_CONFIG ${CMAKE_BINARY_DIR}/NuGet.config)
    set(PACKAGES_CONFIG ${CMAKE_BINARY_DIR}/packages.config)
    get_filename_component(PACKAGES_DIR ${CMAKE_BINARY_DIR}/packages ABSOLUTE)

    # Restore nuget packages
    add_custom_command(
      OUTPUT ${target_dependency}
      DEPENDS ${PACKAGES_CONFIG} ${NUGET_CONFIG}
      COMMAND ${CMAKE_BINARY_DIR}/nuget_exe/src/nuget restore ${PACKAGES_CONFIG} -PackagesDirectory ${PACKAGES_DIR} -ConfigFile ${NUGET_CONFIG}
      VERBATIM)

    add_custom_target(${nuget_target} DEPENDS ${target_dependency})
    add_dependencies(${nuget_target} nuget_exe)
endfunction()

function(kraken_import_nuget_packages
  proj_path
)

set(NUGET_PACKAGES_FILE "${CMAKE_BINARY_DIR}/${proj_path}.vcxproj.user")

file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.NETCore.Platforms.6.0.0-preview.7.21377.19/build/native/Microsoft.NETCore.Platforms"
  MICROSOFT_NETCORE_PLATFORMS
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.CppWinRT.2.0.220912.1/build/native/Microsoft.Windows.CppWinRT"
  MICROSOFT_CPP_WINRT_PROJECT
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.1.1.5/build/native/Microsoft.WindowsAppSDK"
  MICROSOFT_APP_SDK
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.UI.Xaml.2.8.2-prerelease.220830001/build/native/Microsoft.UI.Xaml"
  MICROSOFT_UI_XAML
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2.1.0.1369-prerelease/build/native/Microsoft.Web.WebView2"
  MICROSOFT_WEB_WEBVIEW
)

file(TO_CMAKE_PATH 
  "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Redist\\MSVC\\14.32.31326\\x64\\Microsoft.VC143.CRT"
  MICROSOFT_VC143_CRT
)
file(TO_CMAKE_PATH 
  "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Redist\\MSVC\\14.32.31326\\x64\\Microsoft.VC143.CRT"
  MICROSOFT_VC143_APP_CRT
)
file(TO_CMAKE_PATH
  $ENV{RMANTREE}
  RENDERMAN_LOCATION
)

file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/source/creator/ChaosEngine"
  GENERATED_KRAKEN_SDK
)

if(${proj_path} STREQUAL "source/creator/kraken")
  # This is the executable vsproj, so we need to add
  # dependent DLLs here, and only to this vsproj since
  # it will otherwise add DLLs for all the other libs
  # if we are not careful.
  file(WRITE ${NUGET_PACKAGES_FILE} "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Project ToolsVersion=\"15.0\" DefaultTargets=\"Build\" xmlns=\"http:\/\/schemas.microsoft.com\/developer\/msbuild\/2003\">
  <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT}.props\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.props\')\" />
  <ItemGroup>
    <PackageReference Include=\"Microsoft.Windows.CppWinRT\" Version=\"2.0.220912.1\" />
    <PackageReference Include=\"Microsoft.UI.Xaml\" Version=\"2.8.2-prerelease.220830001\" />
    <PackageReference Include=\"Microsoft.Web.WebView2\" Version=\"1.0.1369-prerelease\" />
    <Manifest Include=\"$(ApplicationManifest)\" />
  </ItemGroup>
  <PropertyGroup Label=\"Globals\">
    <WindowsSdkPackageVersion>10.0.22509.0-preview</WindowsSdkPackageVersion>
    <AppxPackage>true</AppxPackage>
    <TargetPlatformIdentifier>UAP</TargetPlatformIdentifier>
    <TargetOsVersion>10.0</TargetOsVersion>
    <TargetFramework>net6.0-windows10.0.22509.0</TargetFramework>
    <UseWindowsSdkPreview>true</UseWindowsSdkPreview>
    <AppxBundle>Always</AppxBundle>
    <AppxBundlePlatforms>x64</AppxBundlePlatforms>
    <AppxPackageSigningEnabled>true</AppxPackageSigningEnabled>
    <PackageCertificateKeyFile>wabianimation.kraken3d.pfx</PackageCertificateKeyFile>
    <MinimumVisualStudioVersion>17.0</MinimumVisualStudioVersion>
    <ApplicationType>Windows Store</ApplicationType>
    <ApplicationTypeRevision>10.0</ApplicationTypeRevision>
    <WindowsTargetPlatformVersion>10.0.22509.0</WindowsTargetPlatformVersion>
    <WindowsTargetPlatformMinVersion>10.0.22000.0</WindowsTargetPlatformMinVersion>
    <UseWinUI>true</UseWinUI>
    <WindowsAppContainer>true</WindowsAppContainer>
    <CharacterSet>Unicode</CharacterSet>
    <EnablePreviewMsixTooling>true</EnablePreviewMsixTooling>
    <CompileAsWinRT>false</CompileAsWinRT>
    <OutputType>WinExe</OutputType>
    <ReunionMergeWin32Manifest>false</ReunionMergeWin32Manifest>
    <CppWinRTOptimized>true</CppWinRTOptimized>
    <MinimalCoreWin>true</MinimalCoreWin>
    <ProjectName>kraken</ProjectName>
    <RootNamespace>Kraken</RootNamespace>
    <CppWinRTRootNamespace>Kraken</CppWinRTRootNamespace>
    <GeneratedFilesDir>ChaosEngine/</GeneratedFilesDir>
    <CppWinRTUsePrefixes>true</CppWinRTUsePrefixes>
    <DefaultLanguage>en-US</DefaultLanguage>
    <PreferredToolArchitecture>x64</PreferredToolArchitecture>
    <CppWinRTProjectLanguage>C++/WinRT</CppWinRTProjectLanguage>
    <PlatformToolset>v143</PlatformToolset>
    <CanReferenceWinRT>true</CanReferenceWinRT>
    <CppWinRTEnabled>true</CppWinRTEnabled>
    <CppWinRTPackage>true</CppWinRTPackage>
    <XamlLanguage>CppWinRT</XamlLanguage>
    <CppWinRTModernIDL>true</CppWinRTModernIDL>
    <CppWinRTGenerateWindowsMetadata>true</CppWinRTGenerateWindowsMetadata>
    <DesktopCompatible>true</DesktopCompatible>
    <WindowsPackageType>MSIX</WindowsPackageType>
    <CppWinRTParameters>-library Kraken</CppWinRTParameters>
  </PropertyGroup>

  <ItemGroup>
    <None Include=\"${CMAKE_BINARY_DIR}/NuGet.config\" />
    <None Include=\"${CMAKE_BINARY_DIR}/packages.config\" />
  </ItemGroup>
  <ItemGroup>

    <!-- ARNOLD RENDER ENGINE -->

    <Content Include=\"${LIBDIR}/arnold/bin/AdClmHub_2.0.0.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/arnold/bin/adlmint.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/arnold/bin/AdpSDKCore.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/arnold/bin/AdpSDKUI.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/arnold/bin/AdskLicensingSDK_5.0.1.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/arnold/bin/ai.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/arnold/bin/optix.6.6.0.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/arnold/bin/UPI2.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- RENDERMAN RENDER ENGINE -->

    <Content Include=\"${RENDERMAN_LOCATION}/bin/libpxrcore.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${RENDERMAN_LOCATION}/bin/libstats.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${RENDERMAN_LOCATION}/lib/libprman.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- ALEMBIC -->

    <Content Include=\"${LIBDIR}/alembic/bin/Alembic.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- HDF5 -->

    <Content Include=\"${LIBDIR}/hdf5/bin/hdf5.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/hdf5/bin/hdf5_cpp.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/hdf5/bin/hdf5_hl.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/hdf5/bin/hdf5_hl_cpp.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- DRACO -->

    <Content Include=\"${LIBDIR}/draco/lib/draco.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- OPENEXR -->

    <Content Include=\"${LIBDIR}/openexr/bin/Iex-3_0.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/openexr/bin/IlmThread-3_0.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/openexr/bin/Imath-3_0.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/openexr/bin/OpenEXR-3_0.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/openexr/bin/OpenEXRUtil-3_0.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- EMBREE -->

    <Content Include=\"${LIBDIR}/embree/bin/embree3.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- BOOST -->

    <Content Include=\"${LIBDIR}/boost/lib/boost_atomic-${BOOST_LIBRARY_SUFFIX}.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/boost/lib/boost_chrono-${BOOST_LIBRARY_SUFFIX}.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/boost/lib/boost_date_time-${BOOST_LIBRARY_SUFFIX}.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/boost/lib/boost_iostreams-${BOOST_LIBRARY_SUFFIX}.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/boost/lib/boost_numpy310-${BOOST_LIBRARY_SUFFIX}.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/boost/lib/boost_program_options-${BOOST_LIBRARY_SUFFIX}.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/boost/lib/boost_regex-${BOOST_LIBRARY_SUFFIX}.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/boost/lib/boost_system-${BOOST_LIBRARY_SUFFIX}.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/boost/lib/boost_thread-${BOOST_LIBRARY_SUFFIX}.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/boost/lib/boost_python310-${BOOST_LIBRARY_SUFFIX}.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/boost/lib/boost_filesystem-${BOOST_LIBRARY_SUFFIX}.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- OPEN SHADING LANGUAGE -->

    <Content Include=\"${LIBDIR}/osl/bin/oslcomp.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content> 
    <Content Include=\"${LIBDIR}/osl/bin/oslexec.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content> 
    <Content Include=\"${LIBDIR}/osl/bin/oslnoise.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content> 
    <Content Include=\"${LIBDIR}/osl/bin/oslquery.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>  

    <!-- OPENCOLORIO -->

    <Content Include=\"${LIBDIR}/opencolorio/bin/OpenColorIO_2_0.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- OPENIMAGEIO -->

    <Content Include=\"${LIBDIR}/OpenImageIO/bin/OpenImageIO.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/OpenImageIO/bin/OpenImageIO_Util.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content> 

    <!-- OPENIMAGEDENOISE -->

    <Content Include=\"${LIBDIR}/oidn/bin/OpenImageDenoise.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- WALT DISNEY PTEX -->

    <Content Include=\"${LIBDIR}/ptex/bin/Ptex.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- PYTHON -->

    <Content Include=\"${LIBDIR}/python/310/bin/python3.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/python/310/bin/python310.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- OPENVDB -->

    <Content Include=\"${LIBDIR}/openvdb/bin/openvdb.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- BLOSC -->

    <Content Include=\"${LIBDIR}/blosc/bin/blosc.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- TIFF -->

    <Content Include=\"${LIBDIR}/tiff/bin/tiff.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- PNG -->

    <Content Include=\"${LIBDIR}/png/bin/libpng16.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- JPEG -->

    <Content Include=\"${LIBDIR}/jpeg/bin/jpeg62.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <Content Include=\"${LIBDIR}/jpeg/bin/turbojpeg.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- THREADING BUILDING BLOCKS -->

    <Content Include=\"${LIBDIR}/tbb/bin/tbb.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/tbb/bin/tbbmalloc.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/tbb/bin/tbbmalloc_proxy.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- ZLIB -->

    <Content Include=\"${LIBDIR}/zlib/bin/zlib.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- MICROSOFT CRT -->

    <Content Include=\"${MICROSOFT_VC143_CRT}/concrt140.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC143_CRT}/msvcp140.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC143_CRT}/msvcp140_1.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC143_CRT}/msvcp140_2.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC143_CRT}/msvcp140_atomic_wait.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC143_CRT}/msvcp140_codecvt_ids.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC143_CRT}/vccorlib140.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC143_CRT}/vcruntime140.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC143_CRT}/vcruntime140_1.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- MICROSOFT APPLICATION CRT -->

  </ItemGroup>
  <ImportGroup Label=\"ExtensionTargets\">
    <Import Project=\"${MICROSOFT_UI_XAML}.targets\" Condition=\"Exists(\'${MICROSOFT_UI_XAML}.targets\')\" />
    <Import Project=\"${MICROSOFT_WEB_WEBVIEW}.targets\" Condition=\"Exists(\'${MICROSOFT_WEB_WEBVIEW}.targets\')\" />
    <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT}.targets\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.targets\')\" />
  </ImportGroup>
  <Target Name=\"EnsureNuGetPackageBuildImports\" BeforeTargets=\"PrepareForBuild\">
    <PropertyGroup>
      <ErrorText>This project references NuGet package(s) that are missing on this computer. Use NuGet Package Restore to download them.  For more information, see http://go.microsoft.com/fwlink/?LinkID=322105. The missing file is {0}.</ErrorText>
    </PropertyGroup>
    <Error Condition=\"!Exists(\'${MICROSOFT_UI_XAML}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_UI_XAML}.targets\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_WEB_WEBVIEW}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_WEB_WEBVIEW}.targets\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_CPP_WINRT_PROJECT}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_CPP_WINRT_PROJECT}.targets\'))\" />
  </Target>
</Project>
")
else()
  file(WRITE ${NUGET_PACKAGES_FILE} "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Project xmlns=\"http:\/\/schemas.microsoft.com\/developer\/msbuild\/2003\">
  <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT}.props\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK}.props\')\" />
  <PropertyGroup Label=\"Globals\">
    <WindowsSdkPackageVersion>10.0.22509.0-preview</WindowsSdkPackageVersion>
    <CompileAsWinRT>false</CompileAsWinRT>
    <CppWinRTOptimized>true</CppWinRTOptimized>
    <MinimalCoreWin>true</MinimalCoreWin>
    <DefaultLanguage>en-US</DefaultLanguage>
    <MinimumVisualStudioVersion>17.0</MinimumVisualStudioVersion>
    <TargetPlatformIdentifier>UAP</TargetPlatformIdentifier>
    <TargetOsVersion>10.0</TargetOsVersion>
    <TargetFramework>net6.0-windows10.0.22509.0</TargetFramework>
    <UseWindowsSdkPreview>true</UseWindowsSdkPreview>
    <ApplicationType>Windows Store</ApplicationType>
    <ApplicationTypeRevision>10.0</ApplicationTypeRevision>
    <UseWinUI>true</UseWinUI>
    <PreferredToolArchitecture>x64</PreferredToolArchitecture>
    <CppWinRTProjectLanguage>C++/WinRT</CppWinRTProjectLanguage>
    <XamlLanguage>CppWinRT</XamlLanguage>
    <CppWinRTModernIDL>true</CppWinRTModernIDL>
    <PlatformToolset>v143</PlatformToolset>
    <CanReferenceWinRT>true</CanReferenceWinRT>
    <DesktopCompatible>true</DesktopCompatible>
  </PropertyGroup>
  <ItemGroup>
    <None Include=\"${CMAKE_BINARY_DIR}/NuGet.config\" />
    <None Include=\"${CMAKE_BINARY_DIR}/packages.config\" />
  </ItemGroup>
  <ImportGroup Label=\"ExtensionTargets\">
    <Import Project=\"${MICROSOFT_APP_SDK}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK}.targets\')\" />
    <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT}.targets\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.targets\')\" />
  </ImportGroup>
  <Target Name=\"EnsureNuGetPackageBuildImports\" BeforeTargets=\"PrepareForBuild\">
    <PropertyGroup>
      <ErrorText>This project references NuGet package(s) that are missing on this computer. Use NuGet Package Restore to download them.  For more information, see http://go.microsoft.com/fwlink/?LinkID=322105. The missing file is {0}.</ErrorText>
    </PropertyGroup>
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK}.targets\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_CPP_WINRT_PROJECT}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_CPP_WINRT_PROJECT}.targets\'))\" />
  </Target>
</Project>
")
endif()

cmake_host_system_information(RESULT _host_name QUERY HOSTNAME)
string(TOUPPER ${_host_name} UPPER_HOSTNAME)
set(WINDOWS_MACHINE_HOSTNAME ${UPPER_HOSTNAME})
set(WINDOWS_USER_NAME $ENV{USERNAME})
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
  set(WINDOWS_CONFIG_MODE "Release")
else()
  set(WINDOWS_CONFIG_MODE "Debug")
endif()
file(TIMESTAMP ${CMAKE_SOURCE_DIR}/release/windows/appx/vs.appxrecipe.in GEN_TIME "%Y-%m-%dT%H:%M:%S")

file(SHA256 ${CMAKE_SOURCE_DIR}/release/windows/appx/vs.appxrecipe.in _current_hash)
if(NOT EXISTS ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/kraken.build.appxrecipe)
  set(APPX_RECIPE_CHECKSUM_HASH ${_current_hash})
  set(DEPENDENT_DLLS "    
    <!-- ARNOLD RENDER ENGINE -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/AdClmHub_2.0.0.dll\">
      <PackagePath>AdClmHub_2.0.0.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/adlmint.dll\">
      <PackagePath>adlmint.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/AdpSDKCore.dll\">
      <PackagePath>AdpSDKCore.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/AdpSDKUI.dll\">
      <PackagePath>AdpSDKUI.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/AdskLicensingSDK_5.0.1.dll\">
      <PackagePath>AdskLicensingSDK_5.0.1.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/ai.dll\">
      <PackagePath>ai.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/optix.6.6.0.dll\">
      <PackagePath>optix.6.6.0.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/UPI2.dll\">
      <PackagePath>UPI2.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- RENDERMAN RENDER ENGINE -->
    
    <AppxPackagedFile Include=\"${RENDERMAN_LOCATION}/bin/libpxrcore.dll\">
      <PackagePath>libpxrcore.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${RENDERMAN_LOCATION}/bin/libstats.dll\">
      <PackagePath>libstats.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${RENDERMAN_LOCATION}/lib/libprman.dll\">
      <PackagePath>libprman.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- ALEMBIC -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/alembic/bin/Alembic.dll\">
      <PackagePath>Alembic.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- HDF5 -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/hdf5/bin/hdf5.dll\">
      <PackagePath>hdf5.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/hdf5/bin/hdf5_cpp.dll\">
      <PackagePath>hdf5_cpp.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/hdf5/bin/hdf5_hl.dll\">
      <PackagePath>hdf5_hl.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/hdf5/bin/hdf5_hl_cpp.dll\">
      <PackagePath>hdf5_hl_cpp.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- DRACO -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/draco/lib/draco.dll\">
      <PackagePath>draco.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- OPENEXR -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/openexr/bin/Iex-3_0.dll\">
      <PackagePath>Iex-3_0.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/openexr/bin/IlmThread-3_0.dll\">
      <PackagePath>IlmThread-3_0.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/openexr/bin/Imath-3_0.dll\">
      <PackagePath>Imath-3_0.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/openexr/bin/OpenEXR-3_0.dll\">
      <PackagePath>OpenEXR-3_0.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/openexr/bin/OpenEXRUtil-3_0.dll\">
      <PackagePath>OpenEXRUtil-3_0.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- EMBREE -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/embree/bin/embree3.dll\">
      <PackagePath>embree3.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- BOOST -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_atomic-${BOOST_LIBRARY_SUFFIX}.dll\">
      <PackagePath>boost_atomic-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_chrono-${BOOST_LIBRARY_SUFFIX}.dll\">
      <PackagePath>boost_chrono-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_date_time-${BOOST_LIBRARY_SUFFIX}.dll\">
      <PackagePath>boost_date_time-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_iostreams-${BOOST_LIBRARY_SUFFIX}.dll\">
      <PackagePath>boost_iostreams-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_numpy310-${BOOST_LIBRARY_SUFFIX}.dll\">
      <PackagePath>boost_numpy310-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_program_options-${BOOST_LIBRARY_SUFFIX}.dll\">
      <PackagePath>boost_program_options-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_regex-${BOOST_LIBRARY_SUFFIX}.dll\">
      <PackagePath>boost_regex-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_system-${BOOST_LIBRARY_SUFFIX}.dll\">
      <PackagePath>boost_system-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_thread-${BOOST_LIBRARY_SUFFIX}.dll\">
      <PackagePath>boost_thread-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_python310-${BOOST_LIBRARY_SUFFIX}.dll\">
      <PackagePath>boost_python310-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_filesystem-${BOOST_LIBRARY_SUFFIX}.dll\">
      <PackagePath>boost_filesystem-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- OPEN SHADING LANGUAGE -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/osl/bin/oslcomp.dll\">
      <PackagePath>oslcomp.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile> 
    <AppxPackagedFile Include=\"${LIBDIR}/osl/bin/oslexec.dll\">
      <PackagePath>oslexec.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile> 
    <AppxPackagedFile Include=\"${LIBDIR}/osl/bin/oslnoise.dll\">
      <PackagePath>oslnoise.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile> 
    <AppxPackagedFile Include=\"${LIBDIR}/osl/bin/oslquery.dll\">
      <PackagePath>oslquery.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>  
    
    <!-- OPENCOLORIO -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/opencolorio/bin/OpenColorIO_2_0.dll\">
      <PackagePath>OpenColorIO_2_0.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- OPENIMAGEIO -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/OpenImageIO/bin/OpenImageIO.dll\">
      <PackagePath>OpenImageIO.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile> 
    <AppxPackagedFile Include=\"${LIBDIR}/OpenImageIO/bin/OpenImageIO_Util.dll\">
      <PackagePath>OpenImageIO_Util.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile> 
    
    <!-- OPENIMAGEDENOISE -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/oidn/bin/OpenImageDenoise.dll\">
      <PackagePath>OpenImageDenoise.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- WALT DISNEY PTEX -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/ptex/bin/Ptex.dll\">
      <PackagePath>Ptex.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>

    <!-- PYTHON -->

    <AppxPackagedFile Include=\"${LIBDIR}/python/310/bin/python3.dll\">
      <PackagePath>python3.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>

    <AppxPackagedFile Include=\"${LIBDIR}/python/310/bin/python310.dll\">
      <PackagePath>python310.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>

    <!-- OPENVDB -->

    <AppxPackagedFile Include=\"${LIBDIR}/openvdb/bin/openvdb.dll\">
      <PackagePath>openvdb.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>

    <!-- BLOSC -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/blosc/bin/blosc.dll\">
      <PackagePath>blosc.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- TIFF -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/tiff/bin/tiff.dll\">
      <PackagePath>tiff.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- PNG -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/png/bin/libpng16.dll\">
      <PackagePath>libpng16.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- JPEG -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/jpeg/bin/jpeg62.dll\">
      <PackagePath>jpeg62.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/jpeg/bin/turbojpeg.dll\">
      <PackagePath>turbojpeg.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- THREADING BUILDING BLOCKS -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/tbb/bin/tbb.dll\">
      <PackagePath>tbb.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/tbb/bin/tbbmalloc.dll\">
      <PackagePath>tbbmalloc.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${LIBDIR}/tbb/bin/tbbmalloc_proxy.dll\">
      <PackagePath>tbbmalloc_proxy.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- ZLIB -->
    
    <AppxPackagedFile Include=\"${LIBDIR}/zlib/bin/zlib.dll\">
      <PackagePath>zlib.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
    <!-- MICROSOFT CRT -->
    
    <AppxPackagedFile Include=\"${MICROSOFT_VC143_CRT}/concrt140.dll\">
      <PackagePath>concrt140.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${MICROSOFT_VC143_CRT}/msvcp140.dll\">
      <PackagePath>msvcp140.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${MICROSOFT_VC143_CRT}/msvcp140_1.dll\">
      <PackagePath>msvcp140_1.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${MICROSOFT_VC143_CRT}/msvcp140_2.dll\">
      <PackagePath>msvcp140_2.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${MICROSOFT_VC143_CRT}/msvcp140_atomic_wait.dll\">
      <PackagePath>msvcp140_atomic_wait.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${MICROSOFT_VC143_CRT}/msvcp140_codecvt_ids.dll\">
      <PackagePath>msvcp140_codecvt_ids.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${MICROSOFT_VC143_CRT}/vccorlib140.dll\">
      <PackagePath>vccorlib140.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${MICROSOFT_VC143_CRT}/vcruntime140.dll\">
      <PackagePath>vcruntime140.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    <AppxPackagedFile Include=\"${MICROSOFT_VC143_CRT}/vcruntime140_1.dll\">
      <PackagePath>vcruntime140_1.dll</PackagePath>
      <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
      <Modified>${GEN_TIME}</Modified>
    </AppxPackagedFile>
    
")

  # configure_file(
  #   ${CMAKE_SOURCE_DIR}/release/windows/appx/vs.appxrecipe.in
  #   ${CMAKE_BINARY_DIR}/bin/${WINDOWS_CONFIG_MODE}/AppX/vs.appxrecipe @ONLY
  # )

  configure_file(
    ${CMAKE_SOURCE_DIR}/release/windows/appx/vs.appxrecipe.in
    ${CMAKE_BINARY_DIR}/bin/${WINDOWS_CONFIG_MODE}/kraken.build.appxrecipe @ONLY
  )

  file(SHA256 ${CMAKE_BINARY_DIR}/bin/${WINDOWS_CONFIG_MODE}/kraken.build.appxrecipe _new_hash)
  set(APPX_RECIPE_CHECKSUM_HASH ${_new_hash} PARENT_SCOPE)
endif()

if(NOT EXISTS ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/AppxManifest.xml)
  configure_file(
    ${CMAKE_SOURCE_DIR}/release/windows/appx/Package.appxmanifest
    ${CMAKE_BINARY_DIR}/bin/${WINDOWS_CONFIG_MODE}/AppxManifest.xml @ONLY
  )
endif()

# if(NOT EXISTS ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/resources.pri)
# execute_process(COMMAND pwsh -ExecutionPolicy Unrestricted -Command "& MakePri.exe new /cf ${CMAKE_BINARY_DIR}/source/creator/kraken.dir/${CMAKE_BUILD_TYPE}/priconfig.xml /pr ${CMAKE_BINARY_DIR}/source/creator/ /in Kraken"
#                 WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
# file(RENAME
#   ${CMAKE_BINARY_DIR}/resources.pri
#   ${CMAKE_BINARY_DIR}/bin/${WINDOWS_CONFIG_MODE}/resources.pri
# )
# endif()
endfunction()

macro(_run_nuget_config)
  configure_file(
    ${CMAKE_SOURCE_DIR}/release/windows/packages.config
    ${CMAKE_BINARY_DIR}/packages.config
    COPYONLY)

  configure_file(
    ${CMAKE_SOURCE_DIR}/release/windows/NuGet.config
    ${CMAKE_BINARY_DIR}/NuGet.config
    COPYONLY)
endmacro()

function(kraken_config_nuget)
  # In case NuGet packages get cleaned out.
  SUBDIRLIST(NUGET_PACKAGES_DIRLIST ${CMAKE_BINARY_DIR}/packages)
  foreach(_apkg ${NUGET_PACKAGES_DIRLIST})
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/packages/${_apkg})
      if(EXISTS ${CMAKE_BINARY_DIR}/packages.config)
        file(REMOVE ${CMAKE_BINARY_DIR}/packages.config)
      endif()
      if(EXISTS ${CMAKE_BINARY_DIR}/NuGet.config)
        file(REMOVE ${CMAKE_BINARY_DIR}/packages.config)
      endif()
    endif()
  endforeach()

  _run_nuget_config()
endfunction()
