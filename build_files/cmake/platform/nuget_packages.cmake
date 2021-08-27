# This will work as a NuGet package manager of sorts,
# providing the dependencies C++/WinRT, the Microsoft
# Windows 11 SDK, Experimental Windows Features as
# well as the WinUI 3 package. This is necessary in
# order to consume these modern new APIs without the
# need for Visual Studio IDE, and continue natively
# building Kraken from within CMake.

function(kraken_import_nuget_packages
  proj_path
)

set(NUGET_PACKAGES_FILE "${CMAKE_BINARY_DIR}/${proj_path}.vcxproj.user")

file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.NETCore.Platforms.6.0.0-preview.7.21377.19/build/native/Microsoft.NETCore.Platforms"
  MICROSOFT_NETCORE_PLATFORMS
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.CppWinRT.2.0.210806.1/build/native/Microsoft.Windows.CppWinRT"
  MICROSOFT_CPP_WINRT_PROJECT
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.DWrite.1.0.0-experimental1/build/Microsoft.WindowsAppSDK.DWrite"
  MICROSOFT_APP_SDK_DWRITE
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.InteractiveExperiences.1.0.0-experimental1/build/native/Microsoft.WindowsAppSDK.InteractiveExperiences"
  MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.WinUI.1.0.0-experimental1/build/native/Microsoft.WindowsAppSDK.WinUI"
  MICROSOFT_APP_SDK_WINUI
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.WinUI.1.0.0-experimental1/buildTransitive/Microsoft.Build.Msix"
  MICROSOFT_APP_SDK_MSIX_PACK
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.Foundation.1.0.0-experimental1/build/native/Microsoft.WindowsAppSDK.Foundation"
  MICROSOFT_APP_SDK_FOUNDATION
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.1.0.0-experimental1/build/native/Microsoft.WindowsAppSDK"
  MICROSOFT_APP_SDK_EXPERIMENTAL
)

file(TO_CMAKE_PATH 
  "C:\\Program Files\\Microsoft Visual Studio\\2022\\Preview\\VC\\Redist\\MSVC\\14.30.30423\\x64\\Microsoft.VC142.CRT"
  MICROSOFT_VC142_CRT
)
file(TO_CMAKE_PATH 
  "C:\\Program Files\\Microsoft Visual Studio\\2022\\Preview\\VC\\Redist\\MSVC\\14.30.30401\\x64\\Microsoft.VC142.CRT"
  MICROSOFT_VC142_APP_CRT
)
file(TO_CMAKE_PATH
  $ENV{RMANTREE}
  RENDERMAN_LOCATION
)

if(${proj_path} STREQUAL "source/creator/kraken")
  # This is the executable vsproj, so we need to add
  # dependent DLLs here, and only to this vsproj since
  # it will otherwise add DLLs for all the other libs
  # if we are not careful.
  file(WRITE ${NUGET_PACKAGES_FILE} "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Project xmlns=\"http:\/\/schemas.microsoft.com\/developer\/msbuild\/2003\">
  <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT}.props\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_DWRITE}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_DWRITE}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_WINUI}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_WINUI}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_FOUNDATION}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_FOUNDATION}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\')\" />
  <PropertyGroup Label=\"Globals\">
    <TargetPlatformIdentifier>UAP</TargetPlatformIdentifier>
    <TargetPlatformMinVersion>10.0.22000.0</TargetPlatformMinVersion>
    <TargetFramework>net6.0-windows10.0.22000.0</TargetFramework>
    <UseWindowsSdkPreview>true</UseWindowsSdkPreview>
    <WindowsSdkPackageVersion>10.0.22000.160-preview</WindowsSdkPackageVersion>
    <AppxPackage>true</AppxPackage>
    <AppxBundle>Always</AppxBundle>
    <AppxBundlePlatforms>x64</AppxBundlePlatforms>
    <MinimumVisualStudioVersion>16.0</MinimumVisualStudioVersion>
    <AppContainerApplication>true</AppContainerApplication>
    <ApplicationType>Windows Store</ApplicationType>
    <ApplicationTypeRevision>10.0</ApplicationTypeRevision>
    <WindowsTargetPlatformVersion Condition=\" \'$(WindowsTargetPlatformVersion)\' == \'\' \">10.0</WindowsTargetPlatformVersion>
    <WindowsTargetPlatformMinVersion>${WINDOWS_SDK_VERSION}</WindowsTargetPlatformMinVersion>
    <UseWinUI>true</UseWinUI>
    <WindowsAppContainer>false</WindowsAppContainer>
    <CharacterSet>Unicode</CharacterSet>
    <EnablePreviewMsixTooling>true</EnablePreviewMsixTooling>
    <CompileAsWinRT>false</CompileAsWinRT>
    <OutputType>WinExe</OutputType>
    <CppWinRTOptimized>true</CppWinRTOptimized>
    <CppWinRTRootNamespaceAutoMerge>true</CppWinRTRootNamespaceAutoMerge>
    <MinimalCoreWin>true</MinimalCoreWin>
    <ProjectName>Kraken</ProjectName>
    <RootNamespace>Kraken</RootNamespace>
    <DefaultLanguage>en-US</DefaultLanguage>
    <PreferredToolArchitecture>x64</PreferredToolArchitecture>
    <CppWinRTProjectLanguage>C++/WinRT</CppWinRTProjectLanguage>
    <PlatformToolset>v143</PlatformToolset>
    <PackageCertificateKeyFile>wabianimation.kraken3d.pfx</PackageCertificateKeyFile>
    <TempCertificateFilePath>wabianimation.kraken3d.pfx</TempCertificateFilePath>
    <CanReferenceWinRT>true</CanReferenceWinRT>
    <CppWinRTEnabled>true</CppWinRTEnabled>
    <CppWinRTPackage>true</CppWinRTPackage>
    <XamlLanguage>CppWinRT</XamlLanguage>
    <CppWinRTModernIDL>true</CppWinRTModernIDL>
    <CppWinRTGenerateWindowsMetadata>true</CppWinRTGenerateWindowsMetadata>
    <DesktopCompatible>true</DesktopCompatible>
    <WindowsPackageType>MSIX</WindowsPackageType>
  </PropertyGroup>
  <ItemGroup>
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
    <Content Include=\"${LIBDIR}/boost/lib/boost_numpy39-${BOOST_LIBRARY_SUFFIX}.dll\">
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
    <Content Include=\"${LIBDIR}/boost/lib/boost_python39-${BOOST_LIBRARY_SUFFIX}.dll\">
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

    <Content Include=\"${LIBDIR}/python/39/bin/python3.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${LIBDIR}/python/39/bin/python39.dll\">
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

    <Content Include=\"${MICROSOFT_VC142_CRT}/concrt140.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_CRT}/msvcp140.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_CRT}/msvcp140_1.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_CRT}/msvcp140_2.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_CRT}/msvcp140_atomic_wait.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_CRT}/msvcp140_codecvt_ids.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_CRT}/vccorlib140.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_CRT}/vcruntime140.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_CRT}/vcruntime140_1.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>

    <!-- MICROSOFT APPLICATION CRT -->

    <Content Include=\"${MICROSOFT_VC142_APP_CRT}/concrt140_app.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_APP_CRT}/msvcp140_app.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_APP_CRT}/msvcp140_1_app.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_APP_CRT}/msvcp140_2_app.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_APP_CRT}/msvcp140_atomic_wait_app.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_APP_CRT}/msvcp140_codecvt_ids_app.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_APP_CRT}/vccorlib140_app.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_APP_CRT}/vcruntime140_app.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include=\"${MICROSOFT_VC142_APP_CRT}/vcruntime140_1_app.dll\">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
  </ItemGroup>
  <ImportGroup Label=\"ExtensionTargets\">
    <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT}.targets\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_DWRITE}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_DWRITE}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_WINUI}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_WINUI}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_FOUNDATION}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_FOUNDATION}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_EXPERIMENTAL}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_EXPERIMENTAL}.targets\')\" />
  </ImportGroup>
  <Target Name=\"EnsureNuGetPackageBuildImports\" BeforeTargets=\"PrepareForBuild\">
    <PropertyGroup>
      <ErrorText>This project references NuGet package(s) that are missing on this computer. Use NuGet Package Restore to download them.  For more information, see http://go.microsoft.com/fwlink/?LinkID=322105. The missing file is {0}.</ErrorText>
    </PropertyGroup>
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_FOUNDATION}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_FOUNDATION}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_FOUNDATION}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_FOUNDATION}.targets\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_WINUI}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_WINUI}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_WINUI}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_WINUI}.targets\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.targets\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_DWRITE}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_DWRITE}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_DWRITE}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_DWRITE}.targets\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_CPP_WINRT_PROJECT}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_CPP_WINRT_PROJECT}.targets\'))\" />
  </Target>
</Project>
")
else()
  file(WRITE ${NUGET_PACKAGES_FILE} "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Project xmlns=\"http:\/\/schemas.microsoft.com\/developer\/msbuild\/2003\">
  <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT}.props\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_DWRITE}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_DWRITE}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_WINUI}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_WINUI}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_FOUNDATION}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_FOUNDATION}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\')\" />
  <PropertyGroup Label=\"Globals\">
    <TargetPlatformIdentifier>UAP</TargetPlatformIdentifier>
    <WindowsTargetPlatformVersion Condition=\" \'$(WindowsTargetPlatformVersion)\' == \'\' \">10.0</WindowsTargetPlatformVersion>
    <WindowsTargetPlatformMinVersion>${WINDOWS_SDK_VERSION}</WindowsTargetPlatformMinVersion>
    <TargetPlatformMinVersion>10.0.22000.0</TargetPlatformMinVersion>
    <TargetFramework>net6.0-windows10.0.22000.0</TargetFramework>
    <UseWindowsSdkPreview>true</UseWindowsSdkPreview>
    <WindowsSdkPackageVersion>10.0.22000.160-preview</WindowsSdkPackageVersion>
    <CompileAsWinRT>false</CompileAsWinRT>
    <CppWinRTOptimized>true</CppWinRTOptimized>
    <CppWinRTRootNamespaceAutoMerge>true</CppWinRTRootNamespaceAutoMerge>
    <MinimalCoreWin>true</MinimalCoreWin>
    <DefaultLanguage>en-US</DefaultLanguage>
    <MinimumVisualStudioVersion>16.0</MinimumVisualStudioVersion>
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
    <None Include=\"${CMAKE_BINARY_DIR}/packages.config\" />
  </ItemGroup>
  <ImportGroup Label=\"ExtensionTargets\">
    <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT}.targets\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_DWRITE}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_DWRITE}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_WINUI}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_WINUI}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_FOUNDATION}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_FOUNDATION}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_EXPERIMENTAL}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_EXPERIMENTAL}.targets\')\" />
  </ImportGroup>
  <Target Name=\"EnsureNuGetPackageBuildImports\" BeforeTargets=\"PrepareForBuild\">
    <PropertyGroup>
      <ErrorText>This project references NuGet package(s) that are missing on this computer. Use NuGet Package Restore to download them.  For more information, see http://go.microsoft.com/fwlink/?LinkID=322105. The missing file is {0}.</ErrorText>
    </PropertyGroup>
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_FOUNDATION}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_FOUNDATION}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_FOUNDATION}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_FOUNDATION}.targets\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_WINUI}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_WINUI}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_WINUI}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_WINUI}.targets\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.targets\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_DWRITE}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_DWRITE}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_APP_SDK_DWRITE}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_APP_SDK_DWRITE}.targets\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.props\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_CPP_WINRT_PROJECT}.props\'))\" />
    <Error Condition=\"!Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.targets\')\" Text=\"$([System.String]::Format(\'$(ErrorText)\', \'${MICROSOFT_CPP_WINRT_PROJECT}.targets\'))\" />
  </Target>
</Project>
")
endif()

# cmake_host_system_information(RESULT _host_name QUERY HOSTNAME)
# string(TOUPPER ${_host_name} UPPER_HOSTNAME)
# set(WINDOWS_MACHINE_HOSTNAME ${UPPER_HOSTNAME})
# set(WINDOWS_USER_NAME $ENV{USERNAME})
# if(KRAKEN_RELEASE_MODE)
#   set(WINDOWS_CONFIG_MODE "Release")
# else()
#   set(WINDOWS_CONFIG_MODE "Debug")
# endif()
# file(TIMESTAMP ${CMAKE_BINARY_DIR}/bin/Release/Kraken.build.appxrecipe GEN_TIME "%Y-%m-%dT%H:%M:%S")

# file(SHA256 ${CMAKE_BINARY_DIR}/bin/Release/Kraken.build.appxrecipe _current_hash)
# if(TRUE)
#   set(APPX_RECIPE_CHECKSUM_HASH ${_current_hash})
#   set(DEPENDENT_DLLS "    
#     <!-- ARNOLD RENDER ENGINE -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/AdClmHub_2.0.0.dll\">
#       <PackagePath>AdClmHub_2.0.0.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/adlmint.dll\">
#       <PackagePath>adlmint.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/AdpSDKCore.dll\">
#       <PackagePath>AdpSDKCore.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/AdpSDKUI.dll\">
#       <PackagePath>AdpSDKUI.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/AdskLicensingSDK_5.0.1.dll\">
#       <PackagePath>AdskLicensingSDK_5.0.1.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/ai.dll\">
#       <PackagePath>ai.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/optix.6.6.0.dll\">
#       <PackagePath>optix.6.6.0.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/arnold/bin/UPI2.dll\">
#       <PackagePath>UPI2.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- RENDERMAN RENDER ENGINE -->
    
#     <AppxPackagedFile Include=\"${RENDERMAN_LOCATION}/bin/libpxrcore.dll\">
#       <PackagePath>libpxrcore.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${RENDERMAN_LOCATION}/bin/libstats.dll\">
#       <PackagePath>libstats.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${RENDERMAN_LOCATION}/lib/libprman.dll\">
#       <PackagePath>libprman.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- ALEMBIC -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/alembic/bin/Alembic.dll\">
#       <PackagePath>Alembic.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- HDF5 -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/hdf5/bin/hdf5.dll\">
#       <PackagePath>hdf5.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/hdf5/bin/hdf5_cpp.dll\">
#       <PackagePath>hdf5_cpp.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/hdf5/bin/hdf5_hl.dll\">
#       <PackagePath>hdf5_hl.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/hdf5/bin/hdf5_hl_cpp.dll\">
#       <PackagePath>hdf5_hl_cpp.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- DRACO -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/draco/lib/draco.dll\">
#       <PackagePath>draco.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- OPENEXR -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/openexr/bin/Iex-3_0.dll\">
#       <PackagePath>Iex-3_0.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/openexr/bin/IlmThread-3_0.dll\">
#       <PackagePath>IlmThread-3_0.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/openexr/bin/Imath-3_0.dll\">
#       <PackagePath>Imath-3_0.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/openexr/bin/OpenEXR-3_0.dll\">
#       <PackagePath>OpenEXR-3_0.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/openexr/bin/OpenEXRUtil-3_0.dll\">
#       <PackagePath>OpenEXRUtil-3_0.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- EMBREE -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/embree/bin/embree3.dll\">
#       <PackagePath>embree3.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- BOOST -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_atomic-${BOOST_LIBRARY_SUFFIX}.dll\">
#       <PackagePath>boost_atomic-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_chrono-${BOOST_LIBRARY_SUFFIX}.dll\">
#       <PackagePath>boost_chrono-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_date_time-${BOOST_LIBRARY_SUFFIX}.dll\">
#       <PackagePath>boost_date_time-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_iostreams-${BOOST_LIBRARY_SUFFIX}.dll\">
#       <PackagePath>boost_iostreams-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_numpy39-${BOOST_LIBRARY_SUFFIX}.dll\">
#       <PackagePath>boost_numpy39-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_program_options-${BOOST_LIBRARY_SUFFIX}.dll\">
#       <PackagePath>boost_program_options-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_regex-${BOOST_LIBRARY_SUFFIX}.dll\">
#       <PackagePath>boost_regex-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_system-${BOOST_LIBRARY_SUFFIX}.dll\">
#       <PackagePath>boost_system-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_thread-${BOOST_LIBRARY_SUFFIX}.dll\">
#       <PackagePath>boost_thread-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_python39-${BOOST_LIBRARY_SUFFIX}.dll\">
#       <PackagePath>boost_python39-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/boost/lib/boost_filesystem-${BOOST_LIBRARY_SUFFIX}.dll\">
#       <PackagePath>boost_filesystem-${BOOST_LIBRARY_SUFFIX}.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- OPEN SHADING LANGUAGE -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/osl/bin/oslcomp.dll\">
#       <PackagePath>oslcomp.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile> 
#     <AppxPackagedFile Include=\"${LIBDIR}/osl/bin/oslexec.dll\">
#       <PackagePath>oslexec.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile> 
#     <AppxPackagedFile Include=\"${LIBDIR}/osl/bin/oslnoise.dll\">
#       <PackagePath>oslnoise.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile> 
#     <AppxPackagedFile Include=\"${LIBDIR}/osl/bin/oslquery.dll\">
#       <PackagePath>oslquery.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>  
    
#     <!-- OPENCOLORIO -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/opencolorio/bin/OpenColorIO_2_0.dll\">
#       <PackagePath>OpenColorIO_2_0.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- OPENIMAGEIO -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/OpenImageIO/bin/OpenImageIO.dll\">
#       <PackagePath>OpenImageIO.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile> 
#     <AppxPackagedFile Include=\"${LIBDIR}/OpenImageIO/bin/OpenImageIO_Util.dll\">
#       <PackagePath>OpenImageIO_Util.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile> 
    
#     <!-- OPENIMAGEDENOISE -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/oidn/bin/OpenImageDenoise.dll\">
#       <PackagePath>OpenImageDenoise.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- WALT DISNEY PTEX -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/ptex/bin/Ptex.dll\">
#       <PackagePath>Ptex.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- BLOSC -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/blosc/bin/blosc.dll\">
#       <PackagePath>blosc.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- TIFF -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/tiff/bin/tiff.dll\">
#       <PackagePath>tiff.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- PNG -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/png/bin/libpng16.dll\">
#       <PackagePath>libpng16.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- JPEG -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/jpeg/bin/jpeg62.dll\">
#       <PackagePath>jpeg62.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/jpeg/bin/turbojpeg.dll\">
#       <PackagePath>turbojpeg.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- THREADING BUILDING BLOCKS -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/tbb/bin/tbb.dll\">
#       <PackagePath>tbb.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/tbb/bin/tbbmalloc.dll\">
#       <PackagePath>tbbmalloc.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${LIBDIR}/tbb/bin/tbbmalloc_proxy.dll\">
#       <PackagePath>tbbmalloc_proxy.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- ZLIB -->
    
#     <AppxPackagedFile Include=\"${LIBDIR}/zlib/bin/zlib.dll\">
#       <PackagePath>zlib.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- MICROSOFT CRT -->
    
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_CRT}/concrt140.dll\">
#       <PackagePath>concrt140.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_CRT}/msvcp140.dll\">
#       <PackagePath>msvcp140.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_CRT}/msvcp140_1.dll\">
#       <PackagePath>msvcp140_1.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_CRT}/msvcp140_2.dll\">
#       <PackagePath>msvcp140_2.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_CRT}/msvcp140_atomic_wait.dll\">
#       <PackagePath>msvcp140_atomic_wait.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_CRT}/msvcp140_codecvt_ids.dll\">
#       <PackagePath>msvcp140_codecvt_ids.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_CRT}/vccorlib140.dll\">
#       <PackagePath>vccorlib140.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_CRT}/vcruntime140.dll\">
#       <PackagePath>vcruntime140.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_CRT}/vcruntime140_1.dll\">
#       <PackagePath>vcruntime140_1.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
    
#     <!-- MICROSOFT APPLICATION CRT -->
    
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_APP_CRT}/concrt140_app.dll\">
#       <PackagePath>concrt140_app.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_APP_CRT}/msvcp140_app.dll\">
#       <PackagePath>msvcp140_app.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_APP_CRT}/msvcp140_1_app.dll\">
#       <PackagePath>msvcp140_1_app.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_APP_CRT}/msvcp140_2_app.dll\">
#       <PackagePath>msvcp140_2_app.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_APP_CRT}/msvcp140_atomic_wait_app.dll\">
#       <PackagePath>msvcp140_atomic_wait_app.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_APP_CRT}/msvcp140_codecvt_ids_app.dll\">
#       <PackagePath>msvcp140_codecvt_ids_app.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_APP_CRT}/vccorlib140_app.dll\">
#       <PackagePath>vccorlib140_app.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_APP_CRT}/vcruntime140_app.dll\">
#       <PackagePath>vcruntime140_app.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
#     <AppxPackagedFile Include=\"${MICROSOFT_VC142_APP_CRT}/vcruntime140_1_app.dll\">
#       <PackagePath>vcruntime140_1_app.dll</PackagePath>
#       <ReRegisterAppIfChanged>true</ReRegisterAppIfChanged>
#       <Modified>${GEN_TIME}</Modified>
#     </AppxPackagedFile>
# ")

#   configure_file(
#     ${CMAKE_SOURCE_DIR}/release/windows/appx/vs.appxrecipe.in
#     ${CMAKE_BINARY_DIR}/bin/${WINDOWS_CONFIG_MODE}/AppX/vs.appxrecipe @ONLY
#   )

#   configure_file(
#     ${CMAKE_SOURCE_DIR}/release/windows/appx/vs.appxrecipe.in
#     ${CMAKE_BINARY_DIR}/bin/${WINDOWS_CONFIG_MODE}/Kraken.build.appxrecipe @ONLY
#   )

#   file(SHA256 ${CMAKE_BINARY_DIR}/bin/Release/AppX/vs.appxrecipe _new_hash)
#   set(APPX_RECIPE_CHECKSUM_HASH ${_new_hash} PARENT_SCOPE)
# endif()

file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.CppWinRT.2.0.210806.1/bin/cppwinrt.exe"
  MICROSOFT_WINRT_EXECUTABLE
)
set(WINRT_EXECUTABLE ${MICROSOFT_WINRT_EXECUTABLE} PARENT_SCOPE)
endfunction()

function(kraken_init_nuget)
  # Install Required NuGet Packages to the System.
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.CppWinRT.2.0.210806.1/build/native/Microsoft.Windows.CppWinRT.props")
    file(
      COPY
        ${CMAKE_SOURCE_DIR}/release/windows/packages.config
      DESTINATION
        ${CMAKE_BINARY_DIR}
    )
    execute_process(COMMAND pwsh -ExecutionPolicy Unrestricted -Command "& \"C:/Program Files/devtools/nuget.exe\" install ${CMAKE_BINARY_DIR}/packages.config -OutputDirectory ${CMAKE_BINARY_DIR}/packages"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set(KRAKEN_SLN_NEEDS_RESTORE ON)
  endif()

  if(NOT EXISTS ${CMAKE_BINARY_DIR}/Kraken.sln)
    set(KRAKEN_SLN_NEEDS_RESTORE ON)
  endif()

  # Install Required NuGet Packages to the Kraken project.
  if(KRAKEN_SLN_NEEDS_RESTORE)
    execute_process(COMMAND pwsh -ExecutionPolicy Unrestricted -Command "& \"C:/Program Files/devtools/nuget.exe\" restore ${CMAKE_BINARY_DIR}/Kraken.sln"
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    execute_process(COMMAND pwsh -ExecutionPolicy Unrestricted -Command "& \"C:/Program Files/devtools/nuget.exe\" update ${CMAKE_BINARY_DIR}/Kraken.sln"
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    if(EXISTS ${CMAKE_BINARY_DIR}/Kraken.sln)
      set(KRAKEN_SLN_NEEDS_RESTORE OFF)
    endif()
  endif()
endfunction()
