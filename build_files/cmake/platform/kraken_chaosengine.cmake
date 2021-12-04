# get the current nuget sdk kit directory
get_sdk(sdk_folder sdk_version)
get_sdk_include_folder(${sdk_folder} ${sdk_version} sdk_include_folder)
set(dxcore_header "${sdk_include_folder}/um/dxcore.h")
set(target_folder Kraken/ChaosEngine)
set(kraken_chaosengine_adapter_dir ${CMAKE_SOURCE_DIR}/kraken_chaosengine/adapter)
set(kraken_chaosengine_api_root ${CMAKE_SOURCE_DIR}/kraken_chaosengine/api)
set(kraken_chaosengine_dll_dir ${CMAKE_SOURCE_DIR}/kraken_chaosengine/dll)
set(kraken_chaosengine_lib_dir ${CMAKE_SOURCE_DIR}/kraken_chaosengine/lib)
set(kraken_chaosengine_lib_api_dir ${CMAKE_SOURCE_DIR}/kraken_chaosengine/lib/api)
set(kraken_chaosengine_lib_api_experimental_dir ${CMAKE_SOURCE_DIR}/kraken_chaosengine/lib/api.experimental)
set(kraken_chaosengine_lib_api_image_dir ${CMAKE_SOURCE_DIR}/kraken_chaosengine/lib/api.image)
set(kraken_chaosengine_lib_api_ort_dir ${CMAKE_SOURCE_DIR}/kraken_chaosengine/lib/api.ort)
set(kraken_chaosengine_lib_common_dir ${CMAKE_SOURCE_DIR}/kraken_chaosengine/lib/common)
set(kraken_chaosengine_lib_telemetry_dir ${CMAKE_SOURCE_DIR}/kraken_chaosengine/lib/telemetry)

# Retrieve the version of cppwinrt nuget
package_version(
  Microsoft.Windows.CppWinRT
  MICROSOFT_WINDOWS_CXX_WINRT_VERSION
  ${CMAKE_SOURCE_DIR}/packages.config
)

# Override and use the cppwinrt from NuGet package as opposed to the one in the SDK.
set(kraken_chaosengine_CPPWINRT_EXE_PATH_OVERRIDE 
    ${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.CppWinRT.${CppWinRT_version}/bin/cppwinrt.exe)

# add custom target to fetch the nugets
add_fetch_nuget_target(
  # target name
  RESTORE_NUGET_PACKAGES
  # cppwinrt is the target package
  kraken_chaosengine_CPPWINRT_EXE_PATH_OVERRIDE)



# XXX   UNIVERSAL GRAPHICS FRAMEWORK. NAMESPACE OVERRIDE SECTION.   XXX



set(kraken_chaosengine_is_inbox OFF)
if(kraken_CHAOSENGINE_NAMESPACE_OVERRIDE)

  # The Pixar Universe. AKA. UNIVERSAL GRAPHICS FRAMEWORK. (UGF)

  set(output_name "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}.Universe.ChaosEngine")
  set(experimental_output_name "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}.Universe.ChaosEngine.X")
  set(idl_native_output_name "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}.Universe.ChaosEngine.Microsoft")
  set(idl_native_internal_output_name "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}.Universe.ChaosEngine.Microsoft.Internal")

  if(kraken_CHAOSENGINE_NAMESPACE_OVERRIDE STREQUAL "Pixar")
    set(kraken_chaosengine_midl_defines "/DBUILD_INBOX=1")
    set(kraken_chaosengine_is_inbox ON)
  endif()

  set(kraken_chaosengine_root_ns "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}")
  set(BINARY_NAME "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}.Universe.ChaosEngine.dll")
  set(kraken_chaosengine_api_use_ns_prefix false)
else()

  # The Kraken Universe. AKA. KRAKEN GRAPHICS FRAMEWORK. (KGF)

  set(output_name "Kraken.Universe.ChaosEngine")
  set(experimental_output_name "Kraken.Universe.ChaosEngine.X")
  set(idl_native_output_name "Kraken.Universe.ChaosEngine.Microsoft")
  set(idl_native_internal_output_name "Kraken.Universe.ChaosEngine.Microsoft.Internal")
  set(kraken_chaosengine_midl_defines "/DROOT_NS=Kraken")
  set(kraken_chaosengine_root_ns "Kraken")
  set(BINARY_NAME "Kraken.Universe.ChaosEngine.dll")
  set(kraken_chaosengine_api_use_ns_prefix true)
endif()

