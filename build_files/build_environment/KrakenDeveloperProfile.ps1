# This is a cross platorm (macOS, Windows, Linux)
# shell profile loaded with tools and pretty colors.

# ----------------------------------------------- Versioning. -----

$KRAKEN_BUILDING_VERSION = "UNKNOWN"
$PIXAR_BUILDING_VERSION = "UNKNOWN"
$KRAKEN_DEVELOPMENT_MILESTONE = "Open Metaverse"

# ---------------------------------------------- Environment. -----

# System Paths.
$KrakenGlobalView = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$IsKrakenCreatorInDirectory = './Sources/Kraken'
$IsKrakenSourceInDirectory = './Sources/Kraken'
$IsGitDirectory = './.git'

# Automatically get Kraken version.
if ((Test-Path -Path $KrakenGlobalView) -and (Test-Path -Path "$KrakenGlobalView/Sources/Kraken"))
{
  $KRAKEN_CURRENT_VERSION_LINE = (Get-Content -Path "$KrakenGlobalView/Sources/Kraken/KR/KR.Version.swift" -TotalCount 34)[-1]
  $KRAKEN_BUILDING_VERSION_VECTOR = $KRAKEN_CURRENT_VERSION_LINE.split("Pixar.GfVec3i(")[1].split("))")[0]
  $KRAKEN_BUILDING_VERSION = $KRAKEN_BUILDING_VERSION_VECTOR -replace ', ', '.'
}

# Automatically get Pixar USD version.
if ((Test-Path -Path $KrakenGlobalView) -and (Test-Path -Path "$KrakenGlobalView/../SwiftUSD/Sources/pxr/include/pxr/pxrns.h"))
{
  # we use PXR_MINOR_VERSION as the major version (ex. 24).
  $SWIFTUSD_CURRENT_VERSION_MAJOR_LINE = (Get-Content -Path "$KrakenGlobalView/../SwiftUSD/Sources/pxr/include/pxr/pxrns.h" -TotalCount 18)[-1]
  $SWIFTUSD_CURRENT_VERSION_MAJOR = $SWIFTUSD_CURRENT_VERSION_MAJOR_LINE.split("#define PXR_MINOR_VERSION ")[1]
  # we use PXR_PATCH_VERSION as the minor version (ex. 8).
  $SWIFTUSD_CURRENT_VERSION_MINOR_LINE = (Get-Content -Path "$KrakenGlobalView/../SwiftUSD/Sources/pxr/include/pxr/pxrns.h" -TotalCount 19)[-1]
  $SWIFTUSD_CURRENT_VERSION_MINOR = $SWIFTUSD_CURRENT_VERSION_MINOR_LINE.split("#define PXR_PATCH_VERSION ")[1]
  # if the minor version is a single digit, we prefix with 0 (ex. 8 -> 08).
  if ($SWIFTUSD_CURRENT_VERSION_MINOR.length -eq 1) {
    $SWIFTUSD_CURRENT_VERSION_MINOR = "0$SWIFTUSD_CURRENT_VERSION_MINOR"
  }

  # finally put them together (ex. 24.08).
  $PIXAR_BUILDING_VERSION = "$SWIFTUSD_CURRENT_VERSION_MAJOR.$SWIFTUSD_CURRENT_VERSION_MINOR"
}

# MacOS Environment.
# since not all paths get scraped on init,
# and it is annoying having to flip between 
# zsh <-> pwsh to populate your env up.
if ($IsMacOS) {
  $ARM_HOMEBREW_PATH = "/opt/homebrew/bin"
  $SWIFT_SH_BUILD_PATH = "/usr/local/bin"
  $RUST_CARGO_BINPATH = "/Users/$env:USER/.cargo/bin"
  $POSTGRES_APP_PATH = "/Applications/Postgres.app/Contents/Versions/latest/bin"
  # $XCODE_TOOLCHAIN_BINPATH = "/Applications/Xcode-beta.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin"

  # Apple Silicon (M1 Chip ---- /opt/homebrew/bin) 
  if (Test-Path -Path $ARM_HOMEBREW_PATH) {
    $env:PATH = '{0}{1}{2}' -f $env:PATH,[IO.Path]::PathSeparator,$ARM_HOMEBREW_PATH
  }

  # swift-sh (Swift will find the swift-sh binary in the path)
  if (Test-Path -Path $SWIFT_SH_BUILD_PATH) {
    $env:PATH = '{0}{1}{2}' -f $env:PATH,[IO.Path]::PathSeparator,$SWIFT_SH_BUILD_PATH
  }

  # swift sourcekit-lsp
  # if (Test-Path -Path $XCODE_TOOLCHAIN_BINPATH) {
  #   $env:PATH = '{0}{1}{2}' -f $env:PATH,[IO.Path]::PathSeparator,$XCODE_TOOLCHAIN_BINPATH
  # }

  # rust cargo
  if (Test-Path -Path $RUST_CARGO_BINPATH) {
    $env:PATH = '{0}{1}{2}' -f $env:PATH,[IO.Path]::PathSeparator,$RUST_CARGO_BINPATH
  }

  # postgres
  if (Test-Path -Path $POSTGRES_APP_PATH) {
    $env:PATH = '{0}{1}{2}' -f $env:PATH,[IO.Path]::PathSeparator,$POSTGRES_APP_PATH
  }
}

# Windows Environment.
if ($IsWindows) {
  $WINDOWS_SWIFTFORMAT_PATH = "$env:USERPROFILE/Wabi/SwiftFormat/.build/x86_64-unknown-windows-msvc/release"

  # swiftformat (built from source)
  #
  # cd ~/
  # mkdir Wabi
  #
  # git clone https://github.com/nicklockwood/SwiftFormat.git
  # cd SwiftFormat
  #
  # swift build -c release
  if (Test-Path -Path $WINDOWS_SWIFTFORMAT_PATH) {
    $env:PATH = '{0}{1}{2}' -f $env:PATH,[IO.Path]::PathSeparator,$WINDOWS_SWIFTFORMAT_PATH
  }
}

# --------------------------------------- Powershell modules. -----

if (Get-Module -ListAvailable -Name PSWriteColor) {
  Import-Module PSWriteColor
} 
else {
  # Automatically installed if it doesn't exist...
  Install-Module PSWriteColor -Confirm:$true -Force
}

if (Get-Module -ListAvailable -Name Terminal-Icons) {
  Import-Module -Name Terminal-Icons
} 
else {
  # Automatically installed if it doesn't exist...
  Install-Module Terminal-Icons -Confirm:$true -Force
}

if (Get-Module -ListAvailable -Name posh-git) {
  Import-Module posh-git
} 
else {
  # Automatically installed if it doesn't exist...
  Install-Module posh-git -Confirm:$true -Force
}

if($IsWindows) {
  $ChocolateyProfile = "$env:ChocolateyInstall\helpers\chocolateyProfile.psm1"
  if (Test-Path($ChocolateyProfile)) {
    Import-Module "$ChocolateyProfile"
  }
  # Only have to run this once.
  # & choco feature enable -n=allowGlobalConfirmation
}

# -------------------------------------- Developer Functions. -----

function RunAndDebugKraken
{
  if ($IsMacOS) {
    # note: you'll need to add this to the file to ~/.lldbinit:
    #
    # command alias run-no-shell process launch -X 0 --
    #
    & lldb -o run-no-shell $KrakenGlobalView/../build_darwin_release/bin/Release/Kraken.app/Contents/MacOS/Kraken -- $args
  }
}

function HopIntoRootDir
{
  if ((Test-Path -Path $KrakenGlobalView)) {
    cd $KrakenGlobalView
  }
}

# Using Blender's script for this...
function AppleBundleAndNotarize
{
  # $KRAKEN_DMG_NAME = "kraken-$KRAKEN_BUILDING_VERSION-macos-arm64.dmg"
  # zsh $KrakenGlobalView/release/darwin/bundle.sh `
  # --source /Users/$env:USER/actions-runner/_work/Kraken/build_darwin_release/bin/Release `
  # --dmg /Users/$env:USER/actions-runner/_work/Kraken/build_darwin_release/$KRAKEN_DMG_NAME `
  # --bundle-id $env:N_BUNDLE_ID `
  # --username $env:N_USERNAME `
  # --password $env:N_PASSWORD `
  # --codesign $env:C_CERT
}

function RunUnrealEngine5WithDebugger
{
  if ($IsMacOS) {
    if ((Test-Path -Path ~/dev/unreal)) {
      Push-Location ~/dev/unreal
      xcrun ./Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor $Args
      Pop-Location

      Push-Location $KrakenGlobalView
      Pop-Location
    }
  }
}

function GenerateUnrealEngine5
{
  $OVERRIDE = $Args[0]

  if ($IsMacOS) {
    if ((Test-Path -Path ~/dev/unreal)) {
      Push-Location ~/dev/unreal

      if ((-not (Test-Path -Path ./UE5.xcworkspace)) -or ($OVERRIDE -eq "regen")) {
        if ((Test-Path -Path ./GenerateProjectFiles.command)) {
          ./GenerateProjectFiles.command
        }
      }

      Pop-Location
    }
  }
}

function CreateTBBCompatibilityHeaders
{
  $targetDir = "$env:USERPROFILE/Wabi/MetaverseKit/Sources/tbb/include/tbb/"

  @("cache_aligned_allocator.h",
    "combinable.h",
    "concurrent_hash_map.h",
    "collaborative_call_once.h",
    "concurrent_priority_queue.h",
    "concurrent_queue.h",
    "concurrent_unordered_map.h",
    "concurrent_unordered_set.h",
    "concurrent_map.h",
    "concurrent_set.h",
    "concurrent_vector.h",
    "enumerable_thread_specific.h",
    "flow_graph.h",
    "global_control.h",
    "info.h",
    "null_mutex.h",
    "null_rw_mutex.h",
    "parallel_for.h",
    "parallel_for_each.h",
    "parallel_invoke.h",
    "parallel_pipeline.h",
    "parallel_reduce.h",
    "parallel_scan.h",
    "parallel_sort.h",
    "partitioner.h",
    "queuing_mutex.h",
    "queuing_rw_mutex.h",
    "spin_mutex.h",
    "spin_rw_mutex.h",
    "task.h",
    "task_arena.h",
    "task_group.h",
    "task_scheduler_observer.h",
    "tbb_allocator.h",
    "tick_count.h",
    "version.h") | foreach-object {
      $targetFile = $targetDir + $_;
      Write-Color -Text "ADDING TBB", "::", "HEADER", ": ", $_, " -> ", "$targetFile" -Color Blue, DarkGray, Yellow, DarkGray, Cyan, DarkGray, Cyan
      New-Item -ItemType File -Path $targetFile -Force;
      Add-Content -Path "$targetFile" -Value "#pragma once`n`n#include <OneTBB/oneapi/tbb/$_>`n"
  }
}

# -----------------------------------
# Removes all GF templates within the
# Pixar USD source for MetaverseKit.
#
# NOTE: We do this as a script vs mucking
# up the Package.swift file with a bunch
# of excludes, we also shouldn't generate
# the templates in the first place, since
# we want them to match what USD ships.
function RemoveUSDGFTemplates
{
  rm ./pxr/base/gf/dualQuat.template.cpp
  rm ./pxr/base/gf/dualQuat.template.h
  rm ./pxr/base/gf/matrix.template.cpp
  rm ./pxr/base/gf/matrix.template.h
  rm ./pxr/base/gf/matrix2.template.cpp
  rm ./pxr/base/gf/matrix2.template.h
  rm ./pxr/base/gf/matrix3.template.cpp
  rm ./pxr/base/gf/matrix3.template.h
  rm ./pxr/base/gf/matrix4.template.cpp
  rm ./pxr/base/gf/matrix4.template.h
  rm ./pxr/base/gf/quat.template.cpp
  rm ./pxr/base/gf/quat.template.h
  rm ./pxr/base/gf/range.template.cpp
  rm ./pxr/base/gf/range.template.h
  rm ./pxr/base/gf/vec.template.cpp
  rm ./pxr/base/gf/vec.template.h
  rm ./pxr/base/gf/wrapDualQuat.template.cpp
  rm ./pxr/base/gf/wrapMatrix.template.cpp
  rm ./pxr/base/gf/wrapMatrix2.template.cpp
  rm ./pxr/base/gf/wrapMatrix3.template.cpp
  rm ./pxr/base/gf/wrapMatrix4.template.cpp
  rm ./pxr/base/gf/wrapQuat.template.cpp
  rm ./pxr/base/gf/wrapRange.template.cpp
  rm ./pxr/base/gf/wrapVec.template.cpp
}

# -----------------------------------
# Convenience command to arbitrarily 
# copy all contents within sourceDir
# and recusrively iterate through all
# subdirectories and copy them into the
# same directory structure within the
# targetDir.
function CpSrcToTarget
{
  $isDryRun = ($Args[0] -eq "dryrun")
  $sourceDir = $Args[1]
  $targetDir = $Args[2]

  if ($sourceDir -eq $null) {
    Write-Color -Text "ERROR: cannot copy src into target, because src is null." -Color Red
    return
  }
  if ($targetDir -eq $null) {
    Write-Color -Text "ERROR: cannot copy src into target, because target is null." -Color Red
    return
  }

  # -----------------------------------
  Push-Location $sourceDir

  Get-ChildItem $sourceDir -recurse | `
  foreach {
    if($isDryRun) {
      Write-Color -Text "DRYRUN", "::", "Copying", ": ", $_.FullName, " -> ", "$targetFile" -Color Blue, DarkGray, Yellow, DarkGray, Cyan, DarkGray, Cyan
    } else {
      Write-Color -Text "RUNNING", "::", "Copying", ": ", $_.FullName, " -> ", "$targetFile" -Color Blue, DarkGray, Yellow, DarkGray, Cyan, DarkGray, Cyan
      $targetFile = $targetDir + $_.FullName.SubString($sourceDir.Length);

      $targetInfo = Get-Item -Path $_.FullName
      if ($targetInfo.Extension -ne "") {
        New-Item -ItemType File -Path $targetFile -Force;
      } else {
        New-Item -ItemType Directory -Path $targetFile -Force;
      }
      Copy-Item $_.FullName -destination $targetFile
      #Remove-Item $_.FullName
    }
  }

  Pop-Location
  # -----------------------------------
}

function MakeMetaversalBoostFramework
{
  Push-Location /Users/$env:USER/Wabi/MetaverseBoostFramework

  Write-Color -Text "BUILDING", "::", "BOOST", ": ", "macOS", " -> ", "arm64/x86_64" -Color Blue, DarkGray, Yellow, DarkGray, Cyan, DarkGray, Cyan
  make SDK=macosx
  Write-Color -Text "BUILDING", "::", "BOOST", ": ", "visionOS", " -> ", "arm64" -Color Blue, DarkGray, Yellow, DarkGray, Cyan, DarkGray, Cyan
  make SDK=xros
  Write-Color -Text "BUILDING", "::", "BOOST", ": ", "visionOS Simulator", " -> ", "arm64" -Color Blue, DarkGray, Yellow, DarkGray, Cyan, DarkGray, Cyan
  make SDK=xrsimulator
  Write-Color -Text "BUILDING", "::", "BOOST", ": ", "iOS", " -> ", "arm64" -Color Blue, DarkGray, Yellow, DarkGray, Cyan, DarkGray, Cyan
  make SDK=iphoneos
  Write-Color -Text "BUILDING", "::", "BOOST", ": ", "iOS Simulator", " -> ", "arm64/x86_64" -Color Blue, DarkGray, Yellow, DarkGray, Cyan, DarkGray, Cyan
  make SDK=iphonesimulator

  Write-Color -Text "BUILDING", "::", "BOOST", ": ", "XCFramework", " -> ", "UNIVERSAL" -Color Blue, DarkGray, Yellow, DarkGray, Cyan, DarkGray, Cyan
  make xcframework

  Pop-Location
}

# -----------------------------------
# Copies all MaterialX headers to an 
# include directory for MetaverseKit.
#
# NOTE: It is not implemented for linux
# and windows, because this will only
# ever really be done by @furby-tm on
# macOS, internal stuff for the sake
# of convenience.
function CopyHeaders
{
  if ($IsMacOS) {
    $sourceDir = $Args[0]
    $targetDir = $Args[1]
    # -----------------------------------
    Push-Location $sourceDir

    # FOR COPYING THE (*.H) HEADERS TO THE INCLUDE DIRECTORY
    Get-ChildItem $sourceDir -filter "*.h" -recurse | `
    foreach {
      $targetFile = $targetDir + $_.FullName.SubString($sourceDir.Length);
      New-Item -ItemType File -Path $targetFile -Force;
      Copy-Item $_.FullName -destination $targetFile
      Remove-Item $_.FullName
    }

    # FOR COPYING THE (*.HPP) HEADERS TO THE INCLUDE DIRECTORY
    Get-ChildItem $sourceDir -filter "*.hpp" -recurse | `
    foreach {
      $targetFile = $targetDir + $_.FullName.SubString($sourceDir.Length);
      New-Item -ItemType File -Path $targetFile -Force;
      Copy-Item $_.FullName -destination $targetFile
      Remove-Item $_.FullName
    }

    # FOR COPYING THE (*.INL) HEADERS TO THE INCLUDE DIRECTORY
    Get-ChildItem $sourceDir -filter "*.inl" -recurse | `
    foreach {
      $targetFile = $targetDir + $_.FullName.SubString($sourceDir.Length);
      New-Item -ItemType File -Path $targetFile -Force;
      Copy-Item $_.FullName -destination $targetFile
      Remove-Item $_.FullName
    }

    Pop-Location
    # -----------------------------------
  } else {
    Write-Color -Text "This command is not yet implemented for linux or microsoft windows." -Color Yellow
  }
}

function SetupUnrealEngine5Dependencies
{
  if ($IsMacOS) {
    Push-Location ~/dev/unreal

    if ((Test-Path -Path ./Setup.command)) {
      ./Setup.command
    }

    Pop-Location
  }
}

function xcprettyfix
{
  # Small fixup for xc | pretty utility on macOS to ensure it shows possible undefined
  # symbols if any are reported at the end of the build. Else we have to resort to opening
  # xcode up or dealing with the vanilla super verbose output of xcodebuild.
  python $KrakenGlobalView/build_files/build_environment/patches/xcpretty_fix.py | xcpretty
}

function BuildUnrealEngine5
{
  $buildaction = "build"
  $NEEDS_DEPENDENCIES = "NONEED"

  if ($IsMacOS) {
    # Wabi uses a custom build of Unreal Engine 5: 
    # https://github.com/Wabi-Studios/UnrealEngine
    # The build is experimental and is not meant
    # for production.
    #
    #  ----------------- Supporting -----
    #  * Metal 3
    #  * macOS 13.0+ Ventura
    #  * Native Apple silicon (m1+)
    #  * Xcode 14+
    #  * Unreal Engine <-> Kraken Link
    #  * Pixar Universal Scene Description
    #
    # It is apart of a more long-term goal of expanding 
    # cross-platform support & combining the capabilities
    # of many platforms & engines into our unified platform.

    if ((Test-Path -Path ~/dev/unreal)) {
      Push-Location ~/dev/unreal

      if (($Args[0] -eq "nuke")) {
        & git clean -fdx
        return
      }

      if (-not ($Args[0] -eq "regen")) {
        if (($Args[0] -eq "build") -or ($Args[0] -eq "install") -or ($Args[0] -eq "clean")) {
          $buildaction = $Args[0]
        }

        if (($Args[0] -eq "deps")) {
          SetupUnrealEngine5Dependencies
        }
      }

      GenerateUnrealEngine5 $Args[0]

      if ((Test-Path -Path ./UE5.xcworkspace)) {
        xcodebuild `
        -project ./Engine/Intermediate/ProjectFiles/UE5.xcodeproj `
        -scheme UE5 `
        -sdk "macosx" `
        -configuration "Development Editor" `
        $buildaction `
        CODE_SIGN_IDENTITY="Developer ID Application: Wabi Animation Studios, Ltd. Co. (UQ9J5QT9DL)" `
        PROVISIONING_PROFILE="foundation.wabi.kraken" `
        OTHER_CODE_SIGN_FLAGS="--keychain /Library/Keychains/System.keychain" `
        GENERATE_INFOPLIST_FILE="YES"
      }

      Pop-Location

      Push-Location $KrakenGlobalView
      Pop-Location
    }
  }
}

# cross compile swift packages for android.
function WabiCrossCompileAndroidSwiftPackage
{
  if(($Args[0] -eq 'build')) {
    if(($Args[1] -eq '-c') -and ($Args[2] -eq 'release')) {
      $targ = $Args[2]
      $hasTarg = $false
      if(($Args[1] -eq '--target')) {
        $targ = $Args[2]
        $hasTarg = $true
      }
      if(($Args[3] -eq '--target')) {
        $targ = $Args[4]
        $hasTarg = $true
      }

      if ($hasTarg) {
        swift build -c release --target $targ `
        --destination /Users/$env:USER/Wabi/swift-android-sdk/android-aarch64.json `
        -Xlinker -rpath="`$ORIGIN/swift-5.10-android-24-sdk/usr/lib/aarch64-linux-android" 2>&1 | xcbeautify --disable-logging
      } else {
        swift build -c release `
        --destination /Users/$env:USER/Wabi/swift-android-sdk/android-aarch64.json `
        -Xlinker -rpath="`$ORIGIN/swift-5.10-android-24-sdk/usr/lib/aarch64-linux-android" 2>&1 | xcbeautify --disable-logging
      }
    } else {
      $targ = $Args[2]
      $hasTarg = $false
      if(($Args[1] -eq '--target')) {
        $targ = $Args[2]
        $hasTarg = $true
      }
      if(($Args[3] -eq '--target')) {
        $targ = $Args[4]
        $hasTarg = $true
      }

      if ($hasTarg) {
        swift build -c debug --target $targ `
        --destination /Users/$env:USER/Wabi/swift-android-sdk/android-aarch64.json `
        -Xlinker -rpath="`$ORIGIN/swift-5.10-android-24-sdk/usr/lib/aarch64-linux-android" 2>&1 | xcbeautify --disable-logging
      } else {
        swift build -c debug `
        --destination /Users/$env:USER/Wabi/swift-android-sdk/android-aarch64.json `
        -Xlinker -rpath="`$ORIGIN/swift-5.10-android-24-sdk/usr/lib/aarch64-linux-android" 2>&1 | xcbeautify --disable-logging
      }
    }
  }
}

function WabiAnimationPreCommitHook 
{
  # hook.
  if(($Args[0] -eq 'commit') -or ($Args[0] -eq 'format')) {

    # clang format.
    # match hybrid CXX style between Google, Pixar, and Blender
    & git diff --cached --name-only | Where-Object {
      $_ -match '(\.cpp)|(\.h)|(\.json)'
    } | ForEach-Object {
      Write-Color -Text "Formatting", ": ", "$_" -Color Yellow, DarkGray, Cyan
      & clang-format -i -verbose -style=file $_ 2>&1>$null
      & git add $_
    }

    if ($Args[0] -eq 'commit') {
      & git $Args
    }

  } else {

    & git $Args
  }
}

function DeployWabiWeb
{
  if (Test-Path -Path $KrakenGlobalView/../../the-swift-collective/theswiftcollective.com) {
    Push-Location $KrakenGlobalView/../../the-swift-collective/theswiftcollective.com

    swift run -c release
    npm install
    npm run build

    # Copy-Item -Path "./Output/*" -Destination "/opt/homebrew/var/www" -Recurse -Force

    Pop-Location
  }
}

function ClangFormatAll
{
  # clang-format (*.cpp) (*.h) (*.json)
  Get-ChildItem -Path . -Recurse | Where-Object {
    $_ -match '(\.cpp)|(\.cc)|(\.c)|(\.hpp)|(\.h)'
  } | ForEach-Object {
    $relativePath = Get-Item $_.FullName | Resolve-Path -Relative
    Write-Color -Text "Formatting", ": ", "$relativePath" -Color Yellow, DarkGray, Cyan
    & clang-format -i -verbose -style=file $relativePath 2>&1>$null
  }
}

Set-Alias clangformat ClangFormatAll


function WabiFormatAll 
{
  if ((Test-Path -Path $IsKrakenCreatorInDirectory) -and (Test-Path -Path $IsKrakenSourceInDirectory)) {
    Push-Location $KrakenGlobalView

    # # clang-format (*.cpp) (*.json)
    Get-ChildItem -Path . -Recurse | Where-Object {
      $_ -match '(\.cpp)|(\.h)|(\.json)'
    } | ForEach-Object {
      $relativePath = Get-Item $_.FullName | Resolve-Path -Relative
      Write-Color -Text "Formatting", ": ", "$relativePath" -Color Yellow, DarkGray, Cyan
      & clang-format -i -verbose -style=file $relativePath 2>&1>$null
    }

    Pop-Location
  }
}

function RunMidlRT {
  & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\midlrt.exe" $env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.FOUNDATION.IDL /I$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\CREATOR /I"$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES" /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\ANCHOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\EDITORS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKLIB /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKERNEL /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\WM /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\LUXO /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\UNIVERSE /I"$env:USERPROFILE\DEV\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PYTHON\310\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTHREADS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\PYTHON /IC:\VULKANSDK\1.3.224.1\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\TBB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\PYTHON\310\INCLUDE /I"$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ALEMBIC\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE\IMATH /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\HDF5\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\DRACO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\MATERIALX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ARNOLD\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\EMBREE\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OSL\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENIMAGEIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENSUBDIV\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTEX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENCOLORIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENVDB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDARNOLD\COMMON /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDCYCLES\MIKKTSPACE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OIDN\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE\THIRD_PARTY\ATOMIC /I"C:\PROGRAM FILES\PIXAR\RENDERMANPROSERVER-24.4\INCLUDE" /metadata_dir "C:\PROGRAM FILES (X86)\WINDOWS KITS\10\REFERENCES\10.0.22621.0\WINDOWS.FOUNDATION.FOUNDATIONCONTRACT\4.0.0.0" /winrt /W1 /nologo /char signed /env x64 /out"$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine" /winmd "$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine\UNMERGED\KRAKEN.FOUNDATION.WINMD" /h "KRAKEN.FOUNDATION.H" /dlldata "NUL" /iid "KRAKEN.FOUNDATION_I.C" /proxy "KRAKEN.FOUNDATION_P.C" /tlb "KRAKEN.FOUNDATION.TLB" /client none /server none /enum_class /ns_prefix /target "NT60"  /nomidl @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.midlrt.rsp  $env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.FOUNDATION.IDL
  & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\midlrt.exe" $env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.UIKIT.IDL /I$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\CREATOR /I"$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES" /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\ANCHOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\EDITORS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKLIB /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKERNEL /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\WM /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\LUXO /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\UNIVERSE /I"$env:USERPROFILE\DEV\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PYTHON\310\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTHREADS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\PYTHON /IC:\VULKANSDK\1.3.224.1\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\TBB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\PYTHON\310\INCLUDE /I"$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ALEMBIC\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE\IMATH /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\HDF5\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\DRACO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\MATERIALX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ARNOLD\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\EMBREE\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OSL\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENIMAGEIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENSUBDIV\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTEX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENCOLORIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENVDB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDARNOLD\COMMON /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDCYCLES\MIKKTSPACE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OIDN\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE\THIRD_PARTY\ATOMIC /I"C:\PROGRAM FILES\PIXAR\RENDERMANPROSERVER-24.4\INCLUDE" /metadata_dir "C:\PROGRAM FILES (X86)\WINDOWS KITS\10\REFERENCES\10.0.22621.0\WINDOWS.FOUNDATION.FOUNDATIONCONTRACT\4.0.0.0" /winrt /W1 /nologo /char signed /env x64 /out"$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine" /winmd "$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine\UNMERGED\KRAKEN.FOUNDATION.WINMD" /h "KRAKEN.FOUNDATION.H" /dlldata "NUL" /iid "KRAKEN.FOUNDATION_I.C" /proxy "KRAKEN.FOUNDATION_P.C" /tlb "KRAKEN.FOUNDATION.TLB" /client none /server none /enum_class /ns_prefix /target "NT60"  /nomidl @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.midlrt.rsp  $env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.UIKIT.IDL
  & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\midlrt.exe" "$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES\XAMLMETADATAPROVIDER.IDL" /I$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\CREATOR /I"$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES" /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\ANCHOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\EDITORS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKLIB /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKERNEL /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\WM /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\LUXO /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\UNIVERSE /I"$env:USERPROFILE\DEV\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PYTHON\310\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTHREADS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\PYTHON /IC:\VULKANSDK\1.3.224.1\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\TBB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\PYTHON\310\INCLUDE /I"$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ALEMBIC\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE\IMATH /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\HDF5\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\DRACO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\MATERIALX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ARNOLD\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\EMBREE\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OSL\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENIMAGEIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENSUBDIV\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTEX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENCOLORIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENVDB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDARNOLD\COMMON /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDCYCLES\MIKKTSPACE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OIDN\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE\THIRD_PARTY\ATOMIC /I"C:\PROGRAM FILES\PIXAR\RENDERMANPROSERVER-24.4\INCLUDE" /metadata_dir "C:\PROGRAM FILES (X86)\WINDOWS KITS\10\REFERENCES\10.0.22621.0\WINDOWS.FOUNDATION.FOUNDATIONCONTRACT\4.0.0.0" /winrt /W1 /nologo /char signed /env x64 /out"$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine" /winmd "$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine\UNMERGED\KRAKEN.FOUNDATION.WINMD" /h "KRAKEN.FOUNDATION.H" /dlldata "NUL" /iid "KRAKEN.FOUNDATION_I.C" /proxy "KRAKEN.FOUNDATION_P.C" /tlb "KRAKEN.FOUNDATION.TLB" /client none /server none /enum_class /ns_prefix /target "NT60"  /nomidl @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.midlrt.rsp  "$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES\XAMLMETADATAPROVIDER.IDL"
  & "$env:USERPROFILE\dev\build_KRAKEN_Release\packages\Microsoft.Windows.CppWinRT.2.0.220912.1\bin\cppwinrt.exe" @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.cppwinrt_ref.rsp -verbose
  & "$env:USERPROFILE\dev\build_KRAKEN_Release\packages\Microsoft.Windows.CppWinRT.2.0.220912.1\bin\cppwinrt.exe" @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.cppwinrt_comp.rsp -verbose
  # & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\mdmerge.exe" @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.mdmerge.rsp
}

function RunDevelopmentReleaseKraken {
  if($IsWindows) {
    & "$env:USERPROFILE\dev\build_KRAKEN_Release\bin\Release\kraken.exe" $args
  }
  if($IsMacOS) {
    & "~/dev/build_darwin_release/bin/Release/Kraken.app/Contents/MacOS/Kraken" $args
  }
  if($IsLinux) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
  }
}

function PackageKraken {
  & "C:\Program Files (x86)\Microsoft Visual Studio\Shared\NuGetPackages\microsoft.windows.sdk.buildtools\10.0.22621.1\bin\10.0.22621.0\x64\makeappx.exe" pack /v /h SHA256 /d "$env:USERPROFILE/dev/build_KRAKEN_Release/bin/Release/AppX" /p "$env:USERPROFILE/dev/build_KRAKEN_Release/bin/Release/Kraken.msix"
  & "C:\Program Files (x86)\Microsoft Visual Studio\Shared\NuGetPackages\microsoft.windows.sdk.buildtools\10.0.22621.1\bin\10.0.22621.0\x64\signtool.exe" sign /fd SHA256 /sha1 7a8d899988d5bd30621475f65922f43d7854a710 "$env:USERPROFILE/dev/build_KRAKEN_Release/bin/Release/Kraken.msix"
}

function InstallKrakenPackage {
  # import-module AppX -usewindowspowershell
  Remove-AppXPackage Kraken_1.50.0.0_x64__k8b6ffsk23gvw
  Add-AppXPackage $env:USERPROFILE/dev/build_KRAKEN_Release/bin/Release/Kraken.msix
}

function RunKraken {
  # # import-module AppX -usewindowspowershell
  # Remove-AppXPackage Kraken_1.50.0.0_x64__k8b6ffsk23gvw
  # Add-AppXPackage $env:USERPROFILE/dev/build_KRAKEN_Release/bin/Release/kraken_1.50.0.0_x64.msix
  # & "kraken"
  & swift run -c release Kraken
}

function RunDevelopmentDebugKraken {
  & swift run -c debug Kraken
}

# Build kraken app icons, eventually cross platoform, currently just macOS.
function CreateKrakenAppIcons
{
  Push-Location $KrakenGlobalView

  if ($IsMacOS) {
    $build_icns_dir = "$KrakenGlobalView/../build_darwin_release/iconsets/Kraken.iconset"
    $build_icns_svg = "$KrakenGlobalView/release/iconfiles/kraken.svg"

    New-Item -ItemType Directory -Path $build_icns_dir -Force

    &inkscape -z --export-filename="$build_icns_dir/icon_16x16.png"      -w   16 -h   16 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_16x16@2x.png"   -w   32 -h   32 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_32x32.png"      -w   32 -h   32 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_32x32@2x.png"   -w   64 -h   64 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_128x128.png"    -w  128 -h  128 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_128x128@2x.png" -w  256 -h  256 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_256x256.png"    -w  256 -h  256 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_256x256@2x.png" -w  512 -h  512 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_512x512.png"    -w  512 -h  512 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_512x512@2x.png" -w 1024 -h 1024 "$build_icns_svg"
    iconutil -c icns "$build_icns_dir"

    Write-Color -Text "Icons Generated: The iconset is located at: $build_icns_dir" -Color Green
  }

  Pop-Location
}

function CreateAppICNSFromSVG
{
  $build_icns_svg = $Args[0]

  if ($IsMacOS) {
    $build_icns_dir = $build_icns_svg.split('.svg')[0] + '.iconset'

    New-Item -ItemType Directory -Path $build_icns_dir -Force

    &inkscape -z --export-filename="$build_icns_dir/icon_16x16.png"      -w   16 -h   16 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_16x16@2x.png"   -w   32 -h   32 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_32x32.png"      -w   32 -h   32 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_32x32@2x.png"   -w   64 -h   64 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_128x128.png"    -w  128 -h  128 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_128x128@2x.png" -w  256 -h  256 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_256x256.png"    -w  256 -h  256 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_256x256@2x.png" -w  512 -h  512 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_512x512.png"    -w  512 -h  512 "$build_icns_svg"
    &inkscape -z --export-filename="$build_icns_dir/icon_512x512@2x.png" -w 1024 -h 1024 "$build_icns_svg"
    iconutil -c icns "$build_icns_dir"

    Write-Color -Text "Icons Generated: The iconset is located at: $build_icns_dir" -Color Green
  }
}

function ReloadDeveloperProfile {
  . $PROFILE
}

function RunKrakenPythonOfficialRelease {
  if($IsWindows) {
    & ~/dev/lib/win64_vc17/python/310/bin/python.exe $args
  }
  if($IsMacOS) {
    if ((Test-Path -Path "~/dev/build_darwin_release/bin/Release/Kraken.app/Contents/Resources/1.50/python/bin/python3.10")) {
      # prefer kraken python...
      & ~/dev/build_darwin_release/bin/Release/Kraken.app/Contents/Resources/1.50/python/bin/python3.10 $args
    } elseif ((Test-Path -Path "~/dev/lib/apple_darwin_arm64/python/bin/python3.10")) {
      # but also a failsafe in between clean builds...
      & ~/dev/lib/apple_darwin_arm64/python/bin/python3.10 $args
    } else {
      # to prevent the "oh no what it happening" scenario.
      Write-Color -Text "Could not find python in either the Kraken installation or your dev environment." -Color Red
    }
  }
  if($IsLinux) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red    
  }
}

function RunKrakenPythonRelease {
  & swift run -c release Kraken
}

function RunKrakenPythonDebug {
  & swift run -c debug Kraken
}

function ConnectKraken {
  if($IsWindows) {
    ssh -i $env:USERPROFILE\.ssh\lightsail_ssh.pub bitnami@3.231.135.196
  }
  if($IsMacOS) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
  }
  if($IsLinux) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red    
  }
}

# CodeSign the Kraken Application.
function KrakenCodeSign {
  $outCertPath = "$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\wabianimation.kraken3d.pfx"
  if (-not(Test-Path -Path "$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\wabianimation.kraken3d.pfx" -PathType Leaf)) {
    Copy-Item "$env:USERPROFILE\wabianimation.kraken3d.pfx" -Destination $outCertPath
  }
  $cert = @(Get-ChildItem -Path 'Cert:\LocalMachine\Root\7a8d899988d5bd30621475f65922f43d7854a710')[0]
  $certBytes = $cert.Export([System.Security.Cryptography.X509Certificates.X509ContentType]::Pfx)
  [System.IO.File]::WriteAllBytes($outCertPath, $certBytes)
}


# CMake calls this during the build to generate all the icons necessary to package
# the Kraken Application for the Microsoft App Store deployment, as well as nicely
# packaging our application for local deployments.
# Typical usage:
#   GenerateKrakenAssets -Path ${CMAKE_SOURCE_DIR}/release/windows/appx/assets/kraken.svg -Destination ${CMAKE_BINARY_DIR}/release/windows/appx/assets
function GenerateKrakenAssets {
  Param(
    [Parameter(Mandatory=$true,ValueFromPipeline=$true)]
    [string]$Path,
    [string]$Destination,
    [int[]]$Altforms = (16, 20, 24, 30, 32, 36, 40, 48, 60, 64, 72, 80, 96, 256),
    [int[]]$Win32IconSizes = (16, 20, 24, 32, 48, 64, 256),
    [switch]$Unplated = $true,
    [float[]]$Scales = (1.0, 1.25, 1.5, 2.0, 4.0),
    [string]$HighContrastPath = "",
    [switch]$UseExistingIntermediates = $false,
    [switch]$KeepIntermediates = $false
  )

  $assetTypes = @(
    [pscustomobject]@{Name="LargeTile"; W=310; H=310; IconSize=96}
    [pscustomobject]@{Name="LockScreenLogo"; W=24; H=24; IconSize=24}
    [pscustomobject]@{Name="SmallTile"; W=71; H=71; IconSize=36}
    [pscustomobject]@{Name="SplashScreen"; W=620; H=300; IconSize=96}
    [pscustomobject]@{Name="Square44x44Logo"; W=44; H=44; IconSize=32}
    [pscustomobject]@{Name="Square150x150Logo"; W=150; H=150; IconSize=48}
    [pscustomobject]@{Name="StoreLogo"; W=50; H=50; IconSize=36}
    [pscustomobject]@{Name="Wide310x150Logo"; W=310; H=150; IconSize=48}
  )

  function CeilToEven ([int]$i) { if ($i % 2 -eq 0) { [int]($i) } else { [int]($i + 1) } }

  $inflatedAssetSizes = $assetTypes | ForEach-Object {
    $as = $_;
    $scales | ForEach-Object {
      [pscustomobject]@{
        Name = $as.Name + ".scale-$($_*100)"
        W = [math]::Round($as.W * $_, [System.MidpointRounding]::ToPositiveInfinity)
        H = [math]::Round($as.H * $_, [System.MidpointRounding]::ToPositiveInfinity)
        IconSize = CeilToEven ($as.IconSize * $_)
      }
    }
  }

  $allAssetSizes = $inflatedAssetSizes + ($Altforms | ForEach-Object {
      [pscustomobject]@{
        Name = "Square44x44Logo.targetsize-${_}"
        W = [int]$_
        H = [int]$_
        IconSize = [int]$_
      }
      If ($Unplated) {
        [pscustomobject]@{
          Name = "Square44x44Logo.targetsize-${_}_altform-unplated"
          W = [int]$_
          H = [int]$_
          IconSize = [int]$_
        }
      }
    })

  # Cross product with the 3 high contrast modes
  $allAssetSizes = $allAssetSizes | ForEach-Object {
      $asset = $_
      ("standard", "black", "white") | ForEach-Object {
          $contrast = $_
          $name = $asset.Name
          If ($contrast -Ne "standard") {
              If ($HighContrastPath -Eq "") {
                  # "standard" is the default, so we can omit it in filenames
                  return
              }
              $name += "_contrast-" + $contrast
          }
          [pscustomobject]@{
              Name = $name
              W = $asset.W
              H = $asset.H
              IconSize = $asset.IconSize
              Contrast = $_
        }
      }
  }

  $allSizes = $allAssetSizes.IconSize | Group-Object | Select-Object -Expand Name

  $TranslatedSVGPath = & wsl wslpath -u ((Get-Item $Path -ErrorAction:Stop).FullName -Replace "\\","/")
  $TranslatedSVGContrastPath = $null
  If ($HighContrastPath -Ne "") {
      $TranslatedSVGContrastPath = & wsl wslpath -u ((Get-Item $HighContrastPath -ErrorAction:Stop).FullName -Replace "\\","/")
  }
  & wsl which inkscape | Out-Null
  If ($LASTEXITCODE -Ne 0) { throw "Inkscape is not installed in WSL" }
  & wsl which convert | Out-Null
  If ($LASTEXITCODE -Ne 0) { throw "imagemagick is not installed in WSL" }

  If (-Not [string]::IsNullOrEmpty($Destination)) {
    New-Item -Type Directory $Destination -EA:Ignore
    $TranslatedOutDir = & wsl wslpath -u ((Get-Item $Destination -EA:Stop).FullName -Replace "\\","/")
  } Else {
    $TranslatedOutDir = "."
  }

  $intermediates = [System.Collections.Concurrent.ConcurrentBag[PSCustomObject]]::new()
  $intermediateFiles = [System.Collections.Concurrent.ConcurrentBag[string]]::new()

  # Generate the base icons
  $allSizes | ForEach-Object -Parallel {
    $sz = $_;

    $destinationNt = $using:Destination
    $destinationWsl = $using:TranslatedOutDir
    $svgStandardWsl = $using:TranslatedSVGPath
    $svgContrastWsl = $using:TranslatedSVGContrastPath

    $intermediateStandardNt = "$destinationNt\_intermediate.standard.$($sz).png"
    $intermediateStandardWsl = "$destinationWsl/_intermediate.standard.$($sz).png"

    If (($using:UseExistingIntermediates -Eq $false) -Or (-Not (Test-Path $intermediateStandardNt))) {
      wsl inkscape -z --export-filename="$intermediateStandardWsl" -w $sz -h $sz $svgStandardWsl 
    } Else {
      Write-Host "Using existing $intermediateStandardNt"
    }

    ($using:intermediateFiles).Add($intermediateStandardNt)
    ($using:intermediates).Add([PSCustomObject]@{
        Contrast = "standard"
        Size = $sz
        PathWSL = $intermediateStandardWsl
    })

    If ($svgContrastWsl -Ne $null) {
      $intermediateBlackNt = "$destinationNt\_intermediate.black.$($sz).png"
      $intermediateWhiteNt = "$destinationNt\_intermediate.white.$($sz).png"
      $intermediateBlackWsl = "$destinationWsl/_intermediate.black.$($sz).png"
      $intermediateWhiteWsl = "$destinationWsl/_intermediate.white.$($sz).png"

      If (($using:UseExistingIntermediates -Eq $false) -Or (-Not (Test-Path $intermediateBlackNt))) {
        wsl inkscape -z --export-filename="$intermediateBlackWsl" -w $sz -h $sz $svgContrastWsl 
      } Else {
        Write-Host "Using existing $intermediateBlackNt"
      }

      If (($using:UseExistingIntermediates -Eq $false) -Or (-Not (Test-Path $intermediateWhiteNt))) {
        # The HC white icon is just a negative image of the HC black one
        wsl convert "$intermediateBlackWsl" -channel RGB -negate "$intermediateWhiteWsl"
      } Else {
        Write-Host "Using existing $intermediateWhiteNt"
      }

      ($using:intermediateFiles).Add($intermediateBlackNt)
      ($using:intermediateFiles).Add($intermediateWhiteNt)
      ($using:intermediates).Add([PSCustomObject]@{
          Contrast = "black"
          Size = $sz
          PathWSL = $intermediateBlackWsl
      })
      ($using:intermediates).Add([PSCustomObject]@{
          Contrast = "white"
          Size = $sz
          PathWSL = $intermediateWhiteWsl
      })
    }
  }

  $intermediates | ? { $_.Size -In $Win32IconSizes } | Group-Object Contrast | ForEach-Object -Parallel {
    $assetName = "kraken.ico"
    If ($_.Name -Ne "standard") {
      $assetName = "kraken_contrast-$($_.Name).ico"
    }
    Write-Host "Producing win32 .ico for contrast=$($_.Name) as $assetName"
    wsl convert $_.Group.PathWSL "$($using:TranslatedOutDir)/$assetName"
  }

  # Once the base icons are done, splat them into the middles of larger canvases.
  $allAssetSizes | ForEach-Object -Parallel {
    $asset = $_
    If ($asset.W -Eq $asset.H -And $asset.IconSize -eq $asset.W) {
      Write-Host "Copying base icon for size=$($asset.IconSize), contrast=$($asset.Contrast) to $($asset.Name)"
      if(Test-Path -Path "${using:Destination}\_intermediate.$($asset.Contrast).$($asset.IconSize).png") {
        Copy-Item "${using:Destination}\_intermediate.$($asset.Contrast).$($asset.IconSize).png" "${using:Destination}\$($asset.Name).png" -Force
      }
    } Else {
      wsl convert "$($using:TranslatedOutDir)/_intermediate.$($asset.Contrast).$($asset.IconSize).png" -gravity center -background transparent -extent "$($asset.W)x$($asset.H)" "$($using:TranslatedOutDir)/$($asset.Name).png"
    }
  }

  If ($KeepIntermediates -Eq $false) {
    $intermediateFiles | ForEach-Object {
      Write-Host "Cleaning up intermediate file $_"
      if(Test-Path -Path $_) {
        Remove-Item $_
      }
    }
  }
}


# CMake will call this command at build time in order to install the
# necessary NuGet packages Kraken needs to build. NuGet is no longer
# optional with the Windows 11 SDK as well as the .NET framework or
# consuming APIs with CppWinRT.
function InstallNugetPackages {
  & "C:/Program Files/devtools/nuget.exe" install $env:USERPROFILE/dev/build_KRAKEN_Release/packages.config -OutputDirectory $env:USERPROFILE/dev/build_KRAKEN_Release/packages
  & "C:/Program Files/devtools/nuget.exe" restore $env:USERPROFILE/dev/build_KRAKEN_Release/Kraken.sln
  & "C:/Program Files/devtools/nuget.exe" update $env:USERPROFILE/dev/build_KRAKEN_Release/Kraken.sln
}

function ShowPrettyGitRevision {
  Write-Output " "
  if ((Test-Path -Path $IsKrakenCreatorInDirectory) -and (Test-Path -Path $IsKrakenSourceInDirectory)) {
    Push-Location $KrakenGlobalView
    $AUTHOR = git show --format="%an`n" -s
    $LATEST_REVISION = git show HEAD~1 --pretty=format:"%h" --no-patch
    $FOR_DATE = git show -s --format=%ci
    $DID_WHAT_ORIGINAL = git log --format=%B -n 1
    # Remove obnoxious 'Merge Pull Request ...' from commit message.
    $DID_WHAT_LONG = "$DID_WHAT_ORIGINAL" -replace "Merge pull request #.*? from .*?/.*?  ", ""
    $DID_WHAT_LENGTH = "$DID_WHAT_LONG".length
    if ($DID_WHAT_LENGTH -gt 50) {
      $DID_WHAT = "$DID_WHAT_LONG".substring(0, 50)
      $DID_WHAT = "$DID_WHAT..."
    } else {
      $DID_WHAT = $DID_WHAT_LONG
    }
    Write-Color -Text "Latest Revision: ", "$LATEST_REVISION ", "$FOR_DATE" -Color Red, Yellow, Green
    Write-Color -Text "      Work Done: ", "$DID_WHAT" -Color Blue, Cyan
    Write-Color -Text "    Authored by: ", "$AUTHOR" -Color Green, DarkMagenta
    Pop-Location
  }
}

function ShowBanner {
  if((Test-Path -Path $IsKrakenCreatorInDirectory) -and (Test-Path -Path $IsKrakenSourceInDirectory)) {
    . {
        $KRAKEN_BANNER = new-object Collections.ArrayList
        $KRAKEN_BANNER.Add("$([char]0x250F)$([char]0x2533)$([char]0x2513)$([char]0x254B)$([char]0x254B)$([char]0x254B)$([char]0x254B)$([char]0x250F)$([char]0x2513)`n")
        $KRAKEN_BANNER.Add("$([char]0x2503)$([char]0x250F)$([char]0x254B)$([char]0x2533)$([char]0x2533)$([char]0x2501)$([char]0x2513)$([char]0x2503)$([char]0x2523)$([char]0x2533)$([char]0x2501)$([char]0x2533)$([char]0x2501)$([char]0x2533)$([char]0x2513)`n")
        $KRAKEN_BANNER.Add("$([char]0x2503)$([char]0x2517)$([char]0x252B)$([char]0x250F)$([char]0x252B)$([char]0x254B)$([char]0x2517)$([char]0x252B)$([char]0x2501)$([char]0x252B)$([char]0x253B)$([char]0x252B)$([char]0x2503)$([char]0x2503)$([char]0x2503)`n")
        $KRAKEN_BANNER.Add("$([char]0x2517)$([char]0x253B)$([char]0x253B)$([char]0x251B)$([char]0x2517)$([char]0x2501)$([char]0x2501)$([char]0x253B)$([char]0x253B)$([char]0x253B)$([char]0x2501)$([char]0x253B)$([char]0x253B)$([char]0x2501)$([char]0x251B)`n")
      } | Out-Null
    Write-Output " "
    Write-Output " "
    Write-Color -Text $KRAKEN_BANNER -Color Yellow, DarkMagenta, Cyan, DarkBlue
    Write-Output " "
    Write-Color -Text "Kraken ", "v$KRAKEN_BUILDING_VERSION ", "| ", "Pixar ", "v$PIXAR_BUILDING_VERSION ", "| ", "$KRAKEN_DEVELOPMENT_MILESTONE" -Color DarkMagenta, Cyan, Yellow, Green, Blue, Yellow, DarkMagenta
    ShowPrettyGitRevision
  }
}

function RefreshConsole {
  clear
  # Also refreshes or 'sources' this profile. ;-;
  . $PROFILE
}

# Shell theme
if($KrakenGlobalView) {
  Push-Location $KrakenGlobalView
  oh-my-posh init pwsh --config './build_files/build_environment/krakentheme.omp.json' | Invoke-Expression
  Pop-Location
}

# stackys fork of swiftpm.
# this fork allows swiftpm targets to depend on products:
# https://github.com/stackotter/swift-package-manager/tree/same_package_product_dependencies
function StackotterPM {
  $ArgRest = ($Args).Where({$_ -ne 'build' -and $_ -ne 'run' -and $_ -ne 'test' -and $_ -ne 'package'})

  if(($Args[0] -eq 'build')) {
    & /Users/$env:USER/Wabi/SwiftPM/.build/arm64-apple-macosx/release/swift-build $ArgRest 2>&1 | xcbeautify --disable-logging
  } elseif(($Args[0] -eq 'run')) {
    & /Users/$env:USER/Wabi/SwiftPM/.build/arm64-apple-macosx/release/swift-run $ArgRest 2>&1 | xcbeautify --disable-logging
  } elseif(($Args[0] -eq 'test')) {
    & /Users/$env:USER/Wabi/SwiftPM/.build/arm64-apple-macosx/release/swift-test $ArgRest 2>&1 | xcbeautify --disable-logging
  } elseif(($Args[0] -eq 'package')) {
    & /Users/$env:USER/Wabi/SwiftPM/.build/arm64-apple-macosx/release/swift-package $ArgRest 2>&1 | xcbeautify --disable-logging
  } else {
    Write-Color -Text "stackypm: available options are (", "build ", "run ", "test ", "or ", "package", ")" -Color Blue, Yellow, Yellow, Yellow, Blue, Yellow, Blue
  }
}

# cleanup a swiftpm environment.
# cd  into '~/Wabi/Utils' and clone the following repo:
# https://github.com/brokenhandsio/swiftpm-cleanup.git
function SwiftPMCleanup {
  $SWIFT_CLEANUP_SCRIPT = "/Users/$env:USER/Wabi/Utils/swiftpm-cleanup/cleanup.swift"
  if (Test-Path -Path $SWIFT_CLEANUP_SCRIPT) {
    & swift $SWIFT_CLEANUP_SCRIPT --path . --clean-derived-data
  } else {
    Write-Color -Text "swiftclean: missing swiftpm-cleanup project at '/Users/$env:USER/Wabi/Utils/swiftpm-cleanup'." -Color Red
  }
}

function BuildOpenUSD
{
  $OPENUSD_DEV_BUILD_SCRIPT = "/Users/$env:USER/Wabi/OpenUSD/build_scripts/build_usd.py"
  $OPENUSD_BUILD_DIR = "/Users/$env:USER/Wabi/buildopenusd"

  if (Test-Path -Path $OPENUSD_DEV_BUILD_SCRIPT) {
    if (!(Test-Path -Path $OPENUSD_BUILD_DIR)) {
      New-Item -ItemType Directory -Path $OPENUSD_BUILD_DIR -Force;
    }

    if(($Args[0] -eq 'release')) {
      python3 $OPENUSD_DEV_BUILD_SCRIPT --tests --build-shared --build-variant release $OPENUSD_BUILD_DIR
    } else {
      python3 $OPENUSD_DEV_BUILD_SCRIPT --tests --build-shared --build-variant debug $OPENUSD_BUILD_DIR
    }
  } else {
    Write-Color -Text "buildusd: missing openusd project at '/Users/$env:USER/Wabi/OpenUSD'." -Color Red
  }
}

# Cross compile swift package for android.
Set-Alias swiftcross WabiCrossCompileAndroidSwiftPackage

# Build the openusd project.
Set-Alias buildusd BuildOpenUSD

# Cleanup a swiftpm environment.
Set-Alias swiftclean SwiftPMCleanup

# Run stackys fork of swiftpm.
Set-Alias stackypm StackotterPM

# Hop Into Kraken Root
Set-Alias krkn HopIntoRootDir

# Run Kraken with debugger console.
Set-Alias dbgkrkn RunAndDebugKraken

# Generate Kraken App Icons.
Set-Alias genicons CreateKrakenAppIcons
Set-Alias iconmake CreateAppICNSFromSVG

# Run Kraken
Set-Alias kraken RunDevelopmentReleaseKraken
Set-Alias kraken_d RunDevelopmentDebugKraken

#Set-Alias python RunKrakenPythonOfficialRelease
#Set-Alias python3 RunKrakenPythonOfficialRelease

# Run Kraken Python
if ($IsWindows) {
  #Set-Alias python_r RunKrakenPythonRelease
  #Set-Alias python_d RunKrakenPythonDebug
}

# Enter Kraken Server
Set-Alias wabiserver ConnectKraken
Set-Alias wabiweb DeployWabiWeb

# Build Unreal Engine 5
Set-Alias build_unreal BuildUnrealEngine5
Set-Alias run_unreal RunUnrealEngine5WithDebugger

# Utility Convenience
Set-Alias xx RefreshConsole
Set-Alias rr ReloadDeveloperProfile
Set-Alias gg WabiAnimationPreCommitHook
Set-Alias wabi_format WabiFormatAll

# Print Pretty ASCII Logo Variant
ShowBanner