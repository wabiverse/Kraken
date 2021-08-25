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
  </PropertyGroup>
  <ItemGroup>
    <None Include=\"${CMAKE_BINARY_DIR}/packages.config\" />
  </ItemGroup>
  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />
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
