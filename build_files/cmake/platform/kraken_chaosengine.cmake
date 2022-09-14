# get the current nuget sdk kit directory
get_sdk(sdk_folder sdk_version)
get_sdk_include_folder(${sdk_folder} ${sdk_version} sdk_include_folder)
set(dxcore_header "${sdk_include_folder}/um/dxcore.h")
set(target_folder WABIAnimation/UniversalGraphicsFramework)
set(universe_plugin_dir ${UNIVERSE_ROOT}/universe/plugin)
set(universe_api_root ${UNIVERSE_ROOT}/universe/api)
set(universe_dll_dir ${UNIVERSE_ROOT}/universe/dll)
set(universe_lib_dir ${UNIVERSE_ROOT}/universe/lib)
set(universe_lib_api_dir ${UNIVERSE_ROOT}/universe/lib/api)
set(universe_lib_api_experimental_dir ${UNIVERSE_ROOT}/universe/lib/api.x)
set(universe_lib_api_image_dir ${UNIVERSE_ROOT}/universe/lib/api.image)
set(universe_lib_api_runtime_dir ${UNIVERSE_ROOT}/universe/lib/api.runtime)
set(universe_lib_common_dir ${UNIVERSE_ROOT}/universe/lib/common)
set(universe_lib_telemetry_dir ${UNIVERSE_ROOT}/universe/lib/telemetry)

# Retrieve the version of cppwinrt nuget
package_version(
  Microsoft.Windows.CppWinRT
  MICROSOFT_WINDOWS_CXX_WINRT_VERSION
  ${CMAKE_SOURCE_DIR}/release/windows/packages.config
)

# Override and use the cppwinrt from NuGet package as opposed to the one in the SDK.
set(universe_CPPWINRT_EXE_PATH_OVERRIDE 
    ${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.CppWinRT.${CppWinRT_version}/bin/cppwinrt.exe)

# add custom target to fetch the nugets
add_fetch_nuget_target(
  # target name
  RESTORE_NUGET_PACKAGES
  # cppwinrt is the target package
  universe_CPPWINRT_EXE_PATH_OVERRIDE)



# XXX   UNIVERSAL GRAPHICS FRAMEWORK. NAMESPACE OVERRIDE SECTION.   XXX



set(universe_is_inbox OFF)
if(kraken_CHAOSENGINE_NAMESPACE_OVERRIDE)

  # The Pixar Universe. AKA. UNIVERSAL GRAPHICS FRAMEWORK. (UGF)

  set(output_name "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}.Universe.ChaosEngine")
  set(experimental_output_name "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}.Universe.ChaosEngine.X")
  set(idl_native_output_name "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}.Universe.ChaosEngine.Microsoft")
  set(idl_native_internal_output_name "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}.Universe.ChaosEngine.Microsoft.Internal")

  if(kraken_CHAOSENGINE_NAMESPACE_OVERRIDE STREQUAL "Pixar")
    set(universe_midl_defines "/DBUILD_INBOX=1")
    set(universe_is_inbox ON)
  endif()

  set(universe_root_ns "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}")
  set(BINARY_NAME "${kraken_CHAOSENGINE_NAMESPACE_OVERRIDE}.Universe.ChaosEngine.dll")
  set(universe_api_use_ns_prefix false)
else()

  # The Kraken Universe. AKA. KRAKEN GRAPHICS FRAMEWORK. (KGF)

  set(output_name "Kraken.Universe.ChaosEngine")
  set(experimental_output_name "Kraken.Universe.ChaosEngine.X")
  set(idl_native_output_name "Kraken.Universe.ChaosEngine.Microsoft")
  set(idl_native_internal_output_name "Kraken.Universe.ChaosEngine.Microsoft.Internal")
  set(universe_midl_defines "/DROOT_NS=Kraken")
  set(universe_root_ns "Kraken")
  set(BINARY_NAME "Kraken.Universe.ChaosEngine.dll")
  set(universe_api_use_ns_prefix true)
endif()

get_filename_component(exclusions "${universe_api_root}/exclusions.txt" ABSOLUTE)
convert_forward_slashes_to_back(${exclusions} CPPWINRT_COMPONENT_EXCLUSION_LIST)

# For winrt idl files:
# 1) the file name must match the casing of the file on disk.
# 2) for winrt idls the casing must match the namespaces within exactly (Pixar.Universe.ChaosEngine).
# target_cppwinrt will attempt to create a winmd with the name and same casing as the supplied
# idl file. If the name of the winmd file does not match the contained namespaces, cppwinrt.exe
# will generate component template files with fully qualified names, which will not match the existing
# generated component files.
#
# For native (Microsoft Windows) idl files there are no casing restrictions.
get_filename_component(winrt_idl "${universe_api_root}/${universe_root_ns}.Universe.ChaosEngine.idl" ABSOLUTE)
get_filename_component(winrt_experimental_idl "${universe_api_root}/${universe_root_ns}.Universe.ChaosEngine.X.idl" ABSOLUTE)
get_filename_component(idl_native "${universe_api_root}/${universe_root_ns}.Universe.ChaosEngine.Microsoft.idl" ABSOLUTE)
get_filename_component(idl_native_internal "${universe_api_root}/${universe_root_ns}.Universe.ChaosEngine.Microsoft.Internal.idl" ABSOLUTE)
get_filename_component(winrt_winmd "${CMAKE_CURRENT_BINARY_DIR}/${universe_root_ns}.Universe.ChaosEngine.winmd" ABSOLUTE)



# XXX   UNIVERSAL GRAPHICS FRAMEWORK. SDK GENERATION.   XXX



add_generate_cppwinrt_sdk_headers_target(UNIVERSAL_GRAPHICS_FRAMEWORK_SDK
  ${sdk_folder}                                      # location of sdk folder
  ${sdk_version}                                     # sdk version
  ${CMAKE_CURRENT_BINARY_DIR}/universal/sdk/include  # output folder relative to CMAKE_BINARY_DIR where the generated sdk will be placed in the
  ${target_folder}                                   # folder where this target will be placed
)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_SDK RESTORE_NUGET_PACKAGES)



# XXX   UNIVERSAL GRAPHICS FRAMEWORK. API GENERATION.   XXX



target_cppwinrt(UNIVERSAL_GRAPHICS_FRAMEWORK_API
  ${winrt_idl}                  # universe winrt idl to compile
  ${output_name}                # outputs name
  ${universe_lib_api_dir}       # location for cppwinrt generated component sources
  ${sdk_folder}                 # location of sdk folder
  ${sdk_version}                # sdk version
  ${target_folder}              # the folder this target will be placed under
  "${universe_midl_defines}"    # the midl compiler defines
  ${universe_api_use_ns_prefix} # set ns_prefix
  ""                            # set additional cppwinrt ref path
)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_API RESTORE_NUGET_PACKAGES)



# XXX   UNIVERSAL GRAPHICS FRAMEWORK. MOONSHOT FACTORY (EXPERIMENTAL X) API GENERATION.   XXX



target_cppwinrt(UNIVERSAL_GRAPHICS_FRAMEWORK_X_API
  ${winrt_experimental_idl}     # universe winrt idl to compile
  ${experimental_output_name}   # outputs name
  ${universe_lib_api_dir}       # location for cppwinrt generated component sources
  ${sdk_folder}                 # location of sdk folder
  ${sdk_version}                # sdk version
  ${target_folder}              # the folder this target will be placed under
  ${universe_midl_defines}      # the midl compiler defines
  ${universe_api_use_ns_prefix} # set ns_prefix
  ${winrt_winmd}                # set additional cppwinrt ref path
)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_X_API RESTORE_NUGET_PACKAGES)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_X_API UNIVERSAL_GRAPHICS_FRAMEWORK_API)



# XXX   UNIVERSAL GRAPHICS FRAMEWORK. NATIVE (MICROSOFT WINDOWS) API GENERATION.   XXX



target_midl(UNIVERSAL_GRAPHICS_FRAMEWORK_MICROSOFT_API
  ${idl_native}              # universe native idl to compile
  ${idl_native_output_name}  # outputs name
  ${sdk_folder}              # location of sdk folder
  ${sdk_version}             # sdk version
  ${target_folder}           # the folder this target will be placed under
  "${universe_midl_defines}" # the midl compiler defines
)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_MICROSOFT_API RESTORE_NUGET_PACKAGES)



# XXX   UNIVERSAL GRAPHICS FRAMEWORK. NATIVE (MICROSOFT WINDOWS) INTERNAL USE GENERATION.   XXX



target_midl(UNIVERSAL_GRAPHICS_FRAMEWORK_MICROSOFT_INTERNAL
  ${idl_native_internal}             # universe internal native idl to compile
  ${idl_native_internal_output_name} # outputs name
  ${sdk_folder}                      # location of sdk folder
  ${sdk_version}                     # sdk version
  ${target_folder}                   # the folder this target will be placed under
  "${universe_midl_defines}"         # the midl compiler defines
)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_MICROSOFT_INTERNAL RESTORE_NUGET_PACKAGES)



####################################
# Add UNIVERSAL GRAPHICS TELEMETRY #
####################################



# Add static library that will be archived/linked for both static/dynamic libraries
kraken_add_lib(UNIVERSAL_GRAPHICS_FRAMEWORK_TELEMETRY_LIBRARY
  ${universe_lib_telemetry_dir}/notifier.h
  ${CMAKE_SOURCE_DIR}/wabi/base/tf/diagnosticMgr.h
  ${universe_lib_telemetry_dir}/notifier.cpp
  ${universe_lib_telemetry_dir}/pch.h
)

# Compiler options
target_compile_features(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE cxx_std_20)
target_compile_options(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE /GR- /await /wd4238)

# Compiler flags
target_compile_definitions(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE UNIVERSE_ROOT_NS=${universe_root_ns})
target_compile_definitions(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE PLATFORM_WINDOWS)
target_compile_definitions(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE _SCL_SECURE_NO_WARNINGS)
target_compile_definitions(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE BINARY_NAME=\"${BINARY_NAME}\")

# Specify the usage of a precompiled header
target_precompiled_header(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY lib/telemetry/pch.h)

# Includes
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE ${CMAKE_CURRENT_BINARY_DIR})                                # universal graphics generated component headers
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/universal/api)                  # universal graphics generated component headers
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/universal/api/comp_generated)   # universal graphics generated component headers
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/universal/sdk/include)          # universal graphics generated sdk headers
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE ${CMAKE_SOURCE_DIR}/common/include)
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE ${universe_lib_telemetry_dir})
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE ${universe_lib_common_dir}/include)
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE ${CMAKE_SOURCE_DIR}/wabi)
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE ${CMAKE_SOURCE_DIR})

# Properties
set_target_properties(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY
  PROPERTIES
  FOLDER
  ${target_folder})

# Add depsgraph
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY UNIVERSAL_GRAPHICS_FRAMEWORK_SDK)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY UNIVERSAL_GRAPHICS_FRAMEWORK_API)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY UNIVERSAL_GRAPHICS_FRAMEWORK_MICROSOFT_API)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY UNIVERSAL_GRAPHICS_FRAMEWORK_MICROSOFT_INTERNAL)

# Add the maelstrom.
target_link_libraries(UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY PRIVATE maelstrom)



##################################
# Add UNIVERSAL GRAPHICS RUNTIME #
##################################



list(APPEND UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_SRC
  ${universe_lib_api_runtime_dir}/include/UniverseRuntime.h
  ${universe_lib_api_runtime_dir}/UniverseRuntime.cpp
  ${universe_lib_api_runtime_dir}/pch.h
)

# Add static library that will be archived/linked for both static/dynamic library
kraken_add_lib(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY ${UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_SRC})

# Compiler options
target_compile_features(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE cxx_std_20)
target_compile_options(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE /GR- /await /wd4238)

# Compiler definitions
target_compile_definitions(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE UNIVERSE_ROOT_NS=${winml_root_ns})
target_compile_definitions(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE PLATFORM_WINDOWS)
target_compile_definitions(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE _SCL_SECURE_NO_WARNINGS)
if (kraken_CHAOSENGINE_NAMESPACE_OVERRIDE STREQUAL "Pixar")
  target_compile_definitions(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE "BUILD_INBOX=1")
endif()

# Specify the usage of a precompiled header
target_precompiled_header(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY lib/api.runtime/pch.h)

# Includes
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/universal/api)                  # universal graphics generated component headers
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/universal/api/comp_generated)   # universal graphics generated component headers
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/universal/sdk/include)          # universal graphics generated sdk headers

target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE ${UNIVERSE_ROOT}/universe)
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE ${universe_lib_api_dir}) # needed for generated headers
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE ${universe_lib_api_runtime_dir})
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE ${universe_lib_common_dir}/include)
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE ${CMAKE_SOURCE_DIR}/wabi)
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE ${CMAKE_SOURCE_DIR})

set_target_properties(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY
  PROPERTIES
  FOLDER
  ${target_folder})

# Add depsgraph
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY UNIVERSAL_GRAPHICS_FRAMEWORK_SDK)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY UNIVERSAL_GRAPHICS_FRAMEWORK_API)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY UNIVERSAL_GRAPHICS_FRAMEWORK_MICROSOFT_API)
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY UNIVERSAL_GRAPHICS_FRAMEWORK_MICROSOFT_INTERNAL)

# Add the maelstrom.
target_link_libraries(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY PRIVATE maelstrom)

# Link libraries.
target_link_libraries(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY INTERFACE UNIVERSAL_GRAPHICS_FRAMEWORK_API)
target_link_libraries(UNIVERSAL_GRAPHICS_FRAMEWORK_RUNTIME_LIBRARY INTERFACE UNIVERSAL_GRAPHICS_FRAMEWORK_PIXAR_NOTIFIER_LIBRARY)



###################################
#  Add UNIVERSAL GRAPHICS PLUGIN  #
###################################



list(APPEND UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_SRC
  ${universe_plugin_dir}/pch.h
  ${universe_plugin_dir}/universe_plugin.h
  ${universe_plugin_dir}/universe_plugin.cpp
)

kraken_add_lib(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY ${UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_SRC})

if (kraken_CHAOSENGINE_NAMESPACE_OVERRIDE STREQUAL "Windows")
  target_compile_definitions(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY PRIVATE "BUILD_INBOX=1")
endif()

# require C++23
set_target_properties(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY PROPERTIES CXX_STANDARD 23)
set_target_properties(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY PROPERTIES CXX_STANDARD_REQUIRED ON)

# Compiler definitions
kraken_add_include_to_target(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY maelstrom) #TODO: kraken incs too
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY PRIVATE ${CMAKE_SOURCE_DIR})
add_dependencies(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY maelstrom)

# Specify the usage of a precompiled header
target_precompiled_header(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY plugin/pch.h)

# Includes
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY PRIVATE ${UNIVERSE_ROOT}/universe)
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY PRIVATE ${universe_plugin_dir})
target_include_directories(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY PRIVATE ${universe_lib_common_dir}/include)

set_target_properties(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY
  PROPERTIES
  FOLDER
  ${target_folder})

# Link libraries
target_link_libraries(UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY PRIVATE maelstrom)

# add it to the multiverse; the "everything of everything",
# monolithic shared library of all the things in our modern
# computer graphics & 3D animation landscape, elevated with
# WinRT superpowers, as we set the world's GPUs on fire.
#
# kidding of course, yet not.
list(APPEND MULTIVERSE_OF_GRAPHICS UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY)
list(APPEND kraken_EXTERNAL_DEPENDENCIES UNIVERSAL_GRAPHICS_FRAMEWORK_PLUGIN_LIBRARY)