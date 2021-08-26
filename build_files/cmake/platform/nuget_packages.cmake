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

# Layout below is messy, because otherwise the generated file will look messy.
file(WRITE ${NUGET_PACKAGES_FILE} "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Project xmlns=\"http:\/\/schemas.microsoft.com\/developer\/msbuild\/2003\">
  <Import Project=\"${MICROSOFT_APP_SDK_MSIX_PACK}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_MSIX_PACK}.props\')\" />
  <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT}.props\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_DWRITE}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_DWRITE}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_WINUI}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_WINUI}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_FOUNDATION}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_FOUNDATION}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\')\" />
  <PropertyGroup Label=\"Globals\">
    <TargetPlatformIdentifier>UAP</TargetPlatformIdentifier>
    <WindowsTargetPlatformVersion Condition=\" \'$(WindowsTargetPlatformVersion)\' == \'\' \">10.0</WindowsTargetPlatformVersion>
    <TargetPlatformMinVersion>10.0.22000.0</TargetPlatformMinVersion>
    <TargetFramework>net6.0-windows10.0.22000.0</TargetFramework>
    <UseWindowsSdkPreview>true</UseWindowsSdkPreview>
    <WindowsSdkPackageVersion>10.0.22000.160-preview</WindowsSdkPackageVersion>
    <AppContainerApplication>false</AppContainerApplication>
  </PropertyGroup>
  <ItemGroup>
    <None Include=\"${CMAKE_BINARY_DIR}/packages.config\" />
  </ItemGroup>
  <ImportGroup Label=\"ExtensionTargets\">
    <Import Project=\"${MICROSOFT_APP_SDK_MSIX_PACK}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_MSIX_PACK}.targets\')\" />
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

</Project>
")

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
