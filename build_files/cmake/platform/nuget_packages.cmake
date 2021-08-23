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
  "${CMAKE_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.Foundation.1.0.0-experimental1/build/native/Microsoft.WindowsAppSDK.Foundation"
  MICROSOFT_APP_SDK_FOUNDATION
)
file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.WindowsAppSDK.1.0.0-experimental1/build/native/Microsoft.WindowsAppSDK"
  MICROSOFT_APP_SDK_EXPERIMENTAL
)

# Layout below is messy, because otherwise the generated file will look messy.
file(WRITE ${NUGET_PACKAGES_FILE} "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Project xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
  <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT}.props\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_DWRITE}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_DWRITE}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_WINUI}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_WINUI}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_FOUNDATION}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_FOUNDATION}.props\')\" />
  <Import Project=\"${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_EXPERIMENTAL}.props\')\" />
  <ImportGroup Label=\"ExtensionTargets\">
    <Import Project=\"${MICROSOFT_CPP_WINRT_PROJECT}.targets\" Condition=\"Exists(\'${MICROSOFT_CPP_WINRT_PROJECT}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_DWRITE}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_DWRITE}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_INTERACTIVE_EXPERIENCES}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_WINUI}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_WINUI}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_FOUNDATION}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_FOUNDATION}.targets\')\" />
    <Import Project=\"${MICROSOFT_APP_SDK_EXPERIMENTAL}.targets\" Condition=\"Exists(\'${MICROSOFT_APP_SDK_EXPERIMENTAL}.targets\')\" />
  </ImportGroup>
</Project>
")

file(TO_CMAKE_PATH
  "${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.CppWinRT.2.0.210806.1/bin/cppwinrt.exe"
  MICROSOFT_WINRT_EXECUTABLE
)
set(WINRT_EXECUTABLE ${MICROSOFT_WINRT_EXECUTABLE} PARENT_SCOPE)
endfunction()


function(kraken_add_midlrt_references
  _midlrt_reference_target
)

  set(MIDLRT_VCXPROJ_TARGET "${CMAKE_BINARY_DIR}/${_midlrt_reference_target}.vcxproj.midlrt.rsp")
  set(MDMERGE_VCXPROJ_TARGET "${CMAKE_BINARY_DIR}/${_midlrt_reference_target}.vcxproj.mdmerge.rsp")

  file(WRITE ${MIDLRT_VCXPROJ_TARGET} "/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.Foundation.1.0.0-experimental1\\lib\\native\\Microsoft.ApplicationModel.Resources.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.InteractiveExperiences.1.0.0-experimental1\\lib\\uap10.0\\Microsoft.Foundation.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.InteractiveExperiences.1.0.0-experimental1\\lib\\uap10.0\\Microsoft.Graphics.DirectX.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.InteractiveExperiences.1.0.0-experimental1\\lib\\uap10.0\\Microsoft.UI.Composition.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.InteractiveExperiences.1.0.0-experimental1\\lib\\uap10.0\\Microsoft.UI.Dispatching.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.InteractiveExperiences.1.0.0-experimental1\\lib\\uap10.0\\Microsoft.UI.Hosting.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.InteractiveExperiences.1.0.0-experimental1\\lib\\uap10.0\\Microsoft.UI.Input.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.WinUI.1.0.0-experimental1\\lib\\uap10.0\\Microsoft.UI.Text.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.InteractiveExperiences.1.0.0-experimental1\\lib\\uap10.0\\Microsoft.UI.Windowing.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.InteractiveExperiences.1.0.0-experimental1\\lib\\uap10.0\\Microsoft.UI.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.WinUI.1.0.0-experimental1\\lib\\uap10.0\\Microsoft.UI.Xaml.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.WinUI.1.0.0-experimental1\\lib\\uap10.0\\Microsoft.Web.WebView2.Core.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.Foundation.1.0.0-experimental1\\lib\\native\\Microsoft.Windows.AppLifecycle.winmd\"
/reference \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.Foundation.1.0.0-experimental1\\lib\\native\\Microsoft.Windows.PushNotifications.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.AI.MachineLearning.MachineLearningContract\\5.0.0.0\\Windows.AI.MachineLearning.MachineLearningContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.AI.MachineLearning.Preview.MachineLearningPreviewContract\\2.0.0.0\\Windows.AI.MachineLearning.Preview.MachineLearningPreviewContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.Calls.Background.CallsBackgroundContract\\3.0.0.0\\Windows.ApplicationModel.Calls.Background.CallsBackgroundContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.Calls.CallsPhoneContract\\6.0.0.0\\Windows.ApplicationModel.Calls.CallsPhoneContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.Calls.CallsVoipContract\\4.0.0.0\\Windows.ApplicationModel.Calls.CallsVoipContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.CommunicationBlocking.CommunicationBlockingContract\\2.0.0.0\\Windows.ApplicationModel.CommunicationBlocking.CommunicationBlockingContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.SocialInfo.SocialInfoContract\\2.0.0.0\\Windows.ApplicationModel.SocialInfo.SocialInfoContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.StartupTaskContract\\3.0.0.0\\Windows.ApplicationModel.StartupTaskContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Devices.Custom.CustomDeviceContract\\1.0.0.0\\Windows.Devices.Custom.CustomDeviceContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Devices.DevicesLowLevelContract\\3.0.0.0\\Windows.Devices.DevicesLowLevelContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Devices.Printers.PrintersContract\\1.0.0.0\\Windows.Devices.Printers.PrintersContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Devices.SmartCards.SmartCardBackgroundTriggerContract\\3.0.0.0\\Windows.Devices.SmartCards.SmartCardBackgroundTriggerContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Devices.SmartCards.SmartCardEmulatorContract\\6.0.0.0\\Windows.Devices.SmartCards.SmartCardEmulatorContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Foundation.FoundationContract\\4.0.0.0\\Windows.Foundation.FoundationContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Foundation.UniversalApiContract\\14.0.0.0\\Windows.Foundation.UniversalApiContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Gaming.XboxLive.StorageApiContract\\1.0.0.0\\Windows.Gaming.XboxLive.StorageApiContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Graphics.Printing3D.Printing3DContract\\4.0.0.0\\Windows.Graphics.Printing3D.Printing3DContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Networking.Connectivity.WwanContract\\2.0.0.0\\Windows.Networking.Connectivity.WwanContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Networking.Sockets.ControlChannelTriggerContract\\3.0.0.0\\Windows.Networking.Sockets.ControlChannelTriggerContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Security.Isolation.IsolatedWindowsEnvironmentContract\\3.0.0.0\\Windows.Security.Isolation.Isolatedwindowsenvironmentcontract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Services.Maps.GuidanceContract\\3.0.0.0\\Windows.Services.Maps.GuidanceContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Services.Maps.LocalSearchContract\\4.0.0.0\\Windows.Services.Maps.LocalSearchContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Services.Store.StoreContract\\4.0.0.0\\Windows.Services.Store.StoreContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Services.TargetedContent.TargetedContentContract\\1.0.0.0\\Windows.Services.TargetedContent.TargetedContentContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Storage.Provider.CloudFilesContract\\6.0.0.0\\Windows.Storage.Provider.CloudFilesContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.System.Profile.ProfileHardwareTokenContract\\1.0.0.0\\Windows.System.Profile.ProfileHardwareTokenContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.System.Profile.ProfileRetailInfoContract\\1.0.0.0\\Windows.System.Profile.ProfileRetailInfoContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.System.Profile.ProfileSharedModeContract\\2.0.0.0\\Windows.System.Profile.ProfileSharedModeContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.System.Profile.SystemManufacturers.SystemManufacturersContract\\3.0.0.0\\Windows.System.Profile.SystemManufacturers.SystemManufacturersContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.System.SystemManagementContract\\7.0.0.0\\Windows.System.SystemManagementContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.UI.UIAutomation.UIAutomationContract\\2.0.0.0\\Windows.UI.UIAutomation.UIAutomationContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.UI.ViewManagement.ViewManagementViewScalingContract\\1.0.0.0\\Windows.UI.ViewManagement.ViewManagementViewScalingContract.winmd\"
/reference \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.UI.Xaml.Core.Direct.XamlDirectContract\\5.0.0.0\\Windows.UI.Xaml.Core.Direct.XamlDirectContract.winmd\"
")

  file(WRITE ${MDMERGE_VCXPROJ_TARGET} "-v -metadata_dir \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.Foundation.1.0.0-experimental1\\lib\\native\\.\"
-metadata_dir \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.InteractiveExperiences.1.0.0-experimental1\\lib\\uap10.0\\.\"
-metadata_dir \"${CMAKE_BINARY_DIR}\\packages\\Microsoft.WindowsAppSDK.WinUI.1.0.0-experimental1\\lib\\uap10.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.AI.MachineLearning.MachineLearningContract\\5.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.AI.MachineLearning.Preview.MachineLearningPreviewContract\\2.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.Calls.Background.CallsBackgroundContract\\3.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.Calls.CallsPhoneContract\\6.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.Calls.CallsVoipContract\\4.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.CommunicationBlocking.CommunicationBlockingContract\\2.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.SocialInfo.SocialInfoContract\\2.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.ApplicationModel.StartupTaskContract\\3.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Devices.Custom.CustomDeviceContract\\1.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Devices.DevicesLowLevelContract\\3.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Devices.Printers.PrintersContract\\1.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Devices.SmartCards.SmartCardBackgroundTriggerContract\\3.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Devices.SmartCards.SmartCardEmulatorContract\\6.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Foundation.FoundationContract\\4.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Foundation.UniversalApiContract\\14.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Gaming.XboxLive.StorageApiContract\\1.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Graphics.Printing3D.Printing3DContract\\4.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Networking.Connectivity.WwanContract\\2.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Networking.Sockets.ControlChannelTriggerContract\\3.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Security.Isolation.IsolatedWindowsEnvironmentContract\\3.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Services.Maps.GuidanceContract\\3.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Services.Maps.LocalSearchContract\\4.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Services.Store.StoreContract\\4.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Services.TargetedContent.TargetedContentContract\\1.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.Storage.Provider.CloudFilesContract\\6.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.System.Profile.ProfileHardwareTokenContract\\1.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.System.Profile.ProfileRetailInfoContract\\1.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.System.Profile.ProfileSharedModeContract\\2.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.System.Profile.SystemManufacturers.SystemManufacturersContract\\3.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.System.SystemManagementContract\\7.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.UI.UIAutomation.UIAutomationContract\\2.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.UI.ViewManagement.ViewManagementViewScalingContract\\1.0.0.0\\.\"
-metadata_dir \"C:\\Program Files (x86)\\Windows Kits\\10\\References\\10.0.22000.0\\Windows.UI.Xaml.Core.Direct.XamlDirectContract\\5.0.0.0\\.\" -i \"${CMAKE_BINARY_DIR}\\${_midlrt_reference_target}\\Unmerged\\App.winmd\"
-i \"${CMAKE_BINARY_DIR}\\${_midlrt_reference_target}\\Unmerged\\MainWindow.winmd\"
-i \"${CMAKE_BINARY_DIR}\\${_midlrt_reference_target}\\Unmerged\\XamlMetaDataProvider.winmd\" -o \"${CMAKE_BINARY_DIR}\\${_midlrt_reference_target}\\Merged\" -partial -n:1
")
endfunction()


function(kraken_winrt_compiler
  _srcname
  _idlsrc
  _xamlsrc
)

  file(TO_CMAKE_PATH
    "C:\\PROGRAM FILES (X86)\\WINDOWS KITS\\10\\REFERENCES\\10.0.22000.0\\WINDOWS.FOUNDATION.FOUNDATIONCONTRACT\\4.0.0.0"
    MICROSOFT_FOUNDATION_CONTRACT
  )
  
  file(TO_CMAKE_PATH
    "${CMAKE_CURRENT_BINARY_DIR}/Generated Files/${_srcname}.xaml.g.h"
    XAML_G_H
  )

  file(TO_CMAKE_PATH
    "${CMAKE_CURRENT_BINARY_DIR}/Generated Files/${_srcname}.xaml.g.hpp"
    XAML_G_HPP
  )
  
  file(TO_CMAKE_PATH
    "${CMAKE_CURRENT_BINARY_DIR}/Generated Files/${_srcname}.g.h"
    G_H
  )

  list(APPEND KRAKEN_AUTOGENERATE_LANGUAGE_PROJECTION
    ${XAML_G_H}
    ${XAML_G_HPP}
    ${G_H}
  )

  set(GENERATION_DRIVER
    ${_idlsrc}
    ${_xamlsrc}
  )
  
  file(TO_CMAKE_PATH
    "${CMAKE_CURRENT_BINARY_DIR}/${_srcname}.vcxproj.midlrt.rsp"
    MIDLRT_RESP_FILE
  )

  # file(TO_CMAKE_PATH
  #   "${CMAKE_BINARY_DIR}\\source\\creator\\kraken.dir\\Release\\unmerged\\${_srcname}.winmd"
  #   MIDL_GEN_WINMD
  # )

  file(TO_CMAKE_PATH
    "${CMAKE_CURRENT_BINARY_DIR}/Generated Files"
    LANGUAGE_PROJECTION
  )

  file(TOUCH ${MIDLRT_RESP_FILE})

  add_custom_command(
    OUTPUT ${KRAKEN_AUTOGENERATE_LANGUAGE_PROJECTION}
    COMMAND ${WINRT_EXECUTABLE} -reference ${MICROSOFT_FOUNDATION_CONTRACT} -output "${LANGUAGE_PROJECTION}" -include ${GENERATION_DRIVER}  @"${MIDLRT_RESP_FILE}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${GENERATION_DRIVER}
    VERBATIM
  )
  set_source_files_properties(${KRAKEN_AUTOGENERATE_LANGUAGE_PROJECTION}
    PROPERTIES GENERATED TRUE
  )

  # ---------------------------------------- Kraken.App -----
  if(${_srcname} STREQUAL "App")
    set(KRAKEN_WINRT_COMPILED_APP
      ${KRAKEN_AUTOGENERATE_LANGUAGE_PROJECTION}
      PARENT_SCOPE
    )
    add_custom_target(App DEPENDS ${KRAKEN_WINRT_COMPILED_APP})
    # set_property(SOURCE ${KRAKEN_WINRT_COMPILED_APP} PROPERTY VS_DEPLOYMENT_CONTENT 1)

  # --------------------------------- Kraken.MainWindow -----
  elseif(${_srcname} STREQUAL "MainWindow")
    set(KRAKEN_WINRT_COMPILED_MAINWINDOW
      ${KRAKEN_AUTOGENERATE_LANGUAGE_PROJECTION}
      PARENT_SCOPE
    )
    add_custom_target(MainWindow DEPENDS ${KRAKEN_WINRT_COMPILED_MAINWINDOW})
    # set_property(SOURCE ${KRAKEN_WINRT_COMPILED_MAINWINDOW} PROPERTY VS_DEPLOYMENT_CONTENT 1)

  # --------------------------------- Kraken.XamlMetaDataProvider -----
  elseif(${_srcname} STREQUAL "XamlMetaDataProvider")

    # set(KRAKEN_AUTOGENERATE_LANGUAGE_PROJECTION
    #   "${CMAKE_BINARY_DIR}/source/creator/Generated Files/${_srcname}.cpp"
    #   # "${CMAKE_BINARY_DIR}/source/creator/Generated Files/${_srcname}.g.cpp"
    #   # "${CMAKE_BINARY_DIR}/source/creator/Generated Files/${_srcname}.g.h"
    #   "${CMAKE_BINARY_DIR}/source/creator/Generated Files/${_srcname}.h"
    # )

    # add_custom_command(
    #   OUTPUT ${KRAKEN_AUTOGENERATE_LANGUAGE_PROJECTION}
    #   COMMAND ${WINRT_EXECUTABLE} -reference ${MICROSOFT_FOUNDATION_CONTRACT} -output "${LANGUAGE_PROJECTION}"  @"${MIDLRT_RESP_FILE}"
    #   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/source/creator
    #   DEPENDS ${GENERATION_DRIVER}
    #   VERBATIM
    # )
    # set_source_files_properties(${KRAKEN_AUTOGENERATE_LANGUAGE_PROJECTION}
    #   PROPERTIES GENERATED TRUE
    # )
    # add_custom_target(
    #   kraken_winrt_metadata_projection
    #   DEPENDS ${KRAKEN_AUTOGENERATE_LANGUAGE_PROJECTION}
    # )
    # set(KRAKEN_WINRT_COMPILED_METADATA
    #   ${KRAKEN_AUTOGENERATE_LANGUAGE_PROJECTION}
    #   PARENT_SCOPE
    # )
    # set_property(SOURCE ${KRAKEN_WINRT_COMPILED_METADATA} PROPERTY VS_DEPLOYMENT_CONTENT 1)

  endif()

endfunction()