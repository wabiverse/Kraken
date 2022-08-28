# This is a cross platorm (macOS, Windows, Linux)
# shell profile loaded with tools and pretty colors.

# ----------------------------------------------- Versioning. -----

$KRAKEN_BUILDING_VERSION_MAJOR = 1
$KRAKEN_BUILDING_VERSION_MINOR = 50
$KRAKEN_BUILDING_VERSION_CYCLE = "a"
$KRAKEN_DEVELOPMENT_MILESTONE = "Initial Release Sprint"
$PIXAR_BUILDING_VERSION = 22.05

# ---------------------------------------------- Environment. -----

# System Paths.
$KrakenGlobalView = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$IsKrakenCreatorInDirectory = './source/creator'
$IsKrakenSourceInDirectory = './source/kraken/ChaosEngine'
$IsGitDirectory = './.git'

# MacOS Environment.
# since not all paths get scraped on init,
# and it is annoying having to flip between 
# zsh <-> pwsh to populate your env up.
if ($IsMacOS) {
  $ARM_HOMEBREW_PATH = "/opt/homebrew/bin"
  $INTEL_HOMEBREW_PATH = "/usr/local/bin"

  # Apple Silicon (M1 Chip ---- /opt/homebrew/bin) 
  if (Test-Path -Path $ARM_HOMEBREW_PATH) {
    $env:PATH = '{0}{1}{2}' -f $env:PATH,[IO.Path]::PathSeparator,$ARM_HOMEBREW_PATH
  
  # Intel Silicon (Old Chip ---- /usr/local/bin)
  } elseif(Test-Path -Path $INTEL_HOMEBREW_PATH) {
    $env:PATH = '{0}{1}{2}' -f $env:PATH,[IO.Path]::PathSeparator,$INTEL_HOMEBREW_PATH
  }

  # Using beta Swift for the latest and greatest CXX interop features.
  # $SWIFT_BETA_DIR = "/Library/Developer/Toolchains/swift-5.6.2-RELEASE.xctoolchain/usr/bin"
  # if (Test-Path -Path $SWIFT_BETA_DIR) {
  #   $env:PATH = '{0}{1}{2}' -f $SWIFT_BETA_DIR,[IO.Path]::PathSeparator,$env:PATH
  # }
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
    & lldb -o run-no-shell $KrakenGlobalView/../build_darwin_release/bin/Release/Kraken.app/Contents/MacOS/Kraken
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
  $KRAKEN_DMG_NAME = "kraken-$KRAKEN_BUILDING_VERSION_MAJOR.$KRAKEN_BUILDING_VERSION_MINOR$KRAKEN_BUILDING_VERSION_CYCLE-macos-arm64.dmg"
  zsh $KrakenGlobalView/release/darwin/bundle.sh `
  --source /Users/furby/actions-runner/_work/Kraken/build_darwin_release/bin/Release `
  --dmg /Users/furby/actions-runner/_work/Kraken/build_darwin_release/$KRAKEN_DMG_NAME `
  --bundle-id $env:N_BUNDLE_ID `
  --username $env:N_USERNAME `
  --password $env:N_PASSWORD `
  --codesign $env:C_CERT
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
  & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22509.0\x64\midlrt.exe" $env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.FOUNDATION.IDL /I$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\CREATOR /I"$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES" /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\ANCHOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\EDITORS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKLIB /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKERNEL /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\WM /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\LUXO /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\UNIVERSE /I"$env:USERPROFILE\DEV\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PYTHON\39\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTHREADS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\PYTHON /IC:\VULKANSDK\1.2.189.2\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\TBB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\PYTHON\39\INCLUDE /I"$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ALEMBIC\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE\IMATH /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\HDF5\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\DRACO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\MATERIALX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ARNOLD\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\EMBREE\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OSL\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENIMAGEIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENSUBDIV\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTEX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENCOLORIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENVDB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDARNOLD\COMMON /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDCYCLES\MIKKTSPACE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OIDN\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE\THIRD_PARTY\ATOMIC /I"C:\PROGRAM FILES\PIXAR\RENDERMANPROSERVER-24.1\INCLUDE" /metadata_dir "C:\PROGRAM FILES (X86)\WINDOWS KITS\10\REFERENCES\10.0.22509.0\WINDOWS.FOUNDATION.FOUNDATIONCONTRACT\4.0.0.0" /winrt /W1 /nologo /char signed /env x64 /out"$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine" /winmd "$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine\UNMERGED\KRAKEN.FOUNDATION.WINMD" /h "KRAKEN.FOUNDATION.H" /dlldata "NUL" /iid "KRAKEN.FOUNDATION_I.C" /proxy "KRAKEN.FOUNDATION_P.C" /tlb "KRAKEN.FOUNDATION.TLB" /client none /server none /enum_class /ns_prefix /target "NT60"  /nomidl @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.midlrt.rsp  $env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.FOUNDATION.IDL
  & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22509.0\x64\midlrt.exe" $env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.UIKIT.IDL /I$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\CREATOR /I"$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES" /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\ANCHOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\EDITORS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKLIB /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKERNEL /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\WM /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\LUXO /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\UNIVERSE /I"$env:USERPROFILE\DEV\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PYTHON\39\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTHREADS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\PYTHON /IC:\VULKANSDK\1.2.189.2\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\TBB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\PYTHON\39\INCLUDE /I"$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ALEMBIC\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE\IMATH /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\HDF5\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\DRACO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\MATERIALX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ARNOLD\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\EMBREE\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OSL\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENIMAGEIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENSUBDIV\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTEX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENCOLORIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENVDB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDARNOLD\COMMON /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDCYCLES\MIKKTSPACE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OIDN\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE\THIRD_PARTY\ATOMIC /I"C:\PROGRAM FILES\PIXAR\RENDERMANPROSERVER-24.1\INCLUDE" /metadata_dir "C:\PROGRAM FILES (X86)\WINDOWS KITS\10\REFERENCES\10.0.22509.0\WINDOWS.FOUNDATION.FOUNDATIONCONTRACT\4.0.0.0" /winrt /W1 /nologo /char signed /env x64 /out"$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine" /winmd "$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine\UNMERGED\KRAKEN.FOUNDATION.WINMD" /h "KRAKEN.FOUNDATION.H" /dlldata "NUL" /iid "KRAKEN.FOUNDATION_I.C" /proxy "KRAKEN.FOUNDATION_P.C" /tlb "KRAKEN.FOUNDATION.TLB" /client none /server none /enum_class /ns_prefix /target "NT60"  /nomidl @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.midlrt.rsp  $env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.UIKIT.IDL
  & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22509.0\x64\midlrt.exe" "$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES\XAMLMETADATAPROVIDER.IDL" /I$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\CREATOR /I"$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES" /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\ANCHOR /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\EDITORS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKLIB /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\KRAKERNEL /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\WM /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\LUXO /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\UNIVERSE /I"$env:USERPROFILE\DEV\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PYTHON\39\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTHREADS\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\SOURCE\KRAKEN\PYTHON /IC:\VULKANSDK\1.2.189.2\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\TBB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\PYTHON\39\INCLUDE /I"$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ALEMBIC\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE\IMATH /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\HDF5\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\DRACO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\MATERIALX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\ARNOLD\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\EMBREE\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OSL\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENIMAGEIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENSUBDIV\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\PTEX\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENCOLORIO\INCLUDE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OPENVDB\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDARNOLD\COMMON /I$env:USERPROFILE\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDCYCLES\MIKKTSPACE /I$env:USERPROFILE\DEV\LIB\WIN64_VC17\OIDN\INCLUDE /I$env:USERPROFILE\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE\THIRD_PARTY\ATOMIC /I"C:\PROGRAM FILES\PIXAR\RENDERMANPROSERVER-24.1\INCLUDE" /metadata_dir "C:\PROGRAM FILES (X86)\WINDOWS KITS\10\REFERENCES\10.0.22509.0\WINDOWS.FOUNDATION.FOUNDATIONCONTRACT\4.0.0.0" /winrt /W1 /nologo /char signed /env x64 /out"$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine" /winmd "$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\ChaosEngine\UNMERGED\KRAKEN.FOUNDATION.WINMD" /h "KRAKEN.FOUNDATION.H" /dlldata "NUL" /iid "KRAKEN.FOUNDATION_I.C" /proxy "KRAKEN.FOUNDATION_P.C" /tlb "KRAKEN.FOUNDATION.TLB" /client none /server none /enum_class /ns_prefix /target "NT60"  /nomidl @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.midlrt.rsp  "$env:USERPROFILE\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES\XAMLMETADATAPROVIDER.IDL"
  & "$env:USERPROFILE\dev\build_KRAKEN_Release\packages\Microsoft.Windows.CppWinRT.2.0.211028.7\bin\cppwinrt.exe" @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.cppwinrt_ref.rsp -verbose
  & "$env:USERPROFILE\dev\build_KRAKEN_Release\packages\Microsoft.Windows.CppWinRT.2.0.211028.7\bin\cppwinrt.exe" @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.cppwinrt_comp.rsp -verbose
  # & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22509.0\x64\mdmerge.exe" @$env:USERPROFILE\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.mdmerge.rsp
}

function RunDevelopmentReleaseKraken {
  if($IsWindows) {
    & "$env:USERPROFILE\dev\build_KRAKEN_Release\bin\Release\kraken.exe" $args
  }
  if($IsMacOS) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
  }
  if($IsLinux) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
  }
}

function PackageKraken {
  & "C:\Program Files (x86)\Microsoft Visual Studio\Shared\NuGetPackages\microsoft.windows.sdk.buildtools\10.0.22414.2000-preview.rs-prerelease\bin\10.0.22414.0\x64\makeappx.exe" pack /v /h SHA256 /d "$env:USERPROFILE/dev/build_KRAKEN_Release/bin/Release/AppX" /p "$env:USERPROFILE/dev/build_KRAKEN_Release/bin/Release/Kraken.msix"
  & "C:\Program Files (x86)\Microsoft Visual Studio\Shared\NuGetPackages\microsoft.windows.sdk.buildtools\10.0.22414.2000-preview.rs-prerelease\bin\10.0.22414.0\x64\signtool.exe" sign /fd SHA256 /sha1 7a8d899988d5bd30621475f65922f43d7854a710 "$env:USERPROFILE/dev/build_KRAKEN_Release/bin/Release/Kraken.msix"
}

function InstallKrakenPackage {
  # import-module AppX -usewindowspowershell
  Remove-AppXPackage Kraken_1.50.0.0_x64__k8b6ffsk23gvw
  Add-AppXPackage $env:USERPROFILE/dev/build_KRAKEN_Release/bin/Release/Kraken.msix
}

function RunKraken {
  # import-module AppX -usewindowspowershell
  Remove-AppXPackage Kraken_1.50.0.0_x64__k8b6ffsk23gvw
  Add-AppXPackage $env:USERPROFILE/dev/build_KRAKEN_Release/bin/Release/kraken_1.50.0.0_x64.msix
  & "kraken"
}

function RunDevelopmentDebugKraken {
  if($IsWindows) {
    & "$env:USERPROFILE\dev\build_KRAKEN_Debug\bin\Debug\kraken.exe" $args
  }
  if($IsMacOS) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
  }
  if($IsLinux) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
  }
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

function ReloadDeveloperProfile {
  . $PROFILE
}

function RunKrakenPythonOfficialRelease {
  if($IsWindows) {
    & "$env:ProgramFiles/Wabi Animation/Kraken $KRAKEN_BUILDING_VERSION_MAJOR.$KRAKEN_BUILDING_VERSION_MINOR/$KRAKEN_BUILDING_VERSION_MAJOR.$KRAKEN_BUILDING_VERSION_MINOR/python/bin/python.exe" $args
  }
  if($IsMacOS) {
    & ~/dev/lib/apple_darwin_arm64/python/bin/python3.10 $args
  }
  if($IsLinux) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red    
  }
}

function RunKrakenPythonRelease {
  if($IsWindows) {
    & "$env:USERPROFILE/dev/build_KRAKEN_Release/bin/Release/$KRAKEN_BUILDING_VERSION_MAJOR.$KRAKEN_BUILDING_VERSION_MINOR/python/bin/python.exe" $args
  }
  if($IsMacOS) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
  }
  if($IsLinux) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red    
  }
}

function RunKrakenPythonDebug {
  if($IsWindows) {
    & "$env:USERPROFILE/dev/build_KRAKEN_Debug/bin/Debug/$KRAKEN_BUILDING_VERSION_MAJOR.$KRAKEN_BUILDING_VERSION_MINOR/python/bin/python_d.exe" $args
  }
  if($IsMacOS) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
  }
  if($IsLinux) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red    
  }
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
      wsl inkscape -z -e "$intermediateStandardWsl" -w $sz -h $sz $svgStandardWsl 
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
        wsl inkscape -z -e "$intermediateBlackWsl" -w $sz -h $sz $svgContrastWsl 
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
      Copy-Item "${using:Destination}\_intermediate.$($asset.Contrast).$($asset.IconSize).png" "${using:Destination}\$($asset.Name).png" -Force
    } Else {
      wsl convert "$($using:TranslatedOutDir)/_intermediate.$($asset.Contrast).$($asset.IconSize).png" -gravity center -background transparent -extent "$($asset.W)x$($asset.H)" "$($using:TranslatedOutDir)/$($asset.Name).png"
    }
  }

  If ($KeepIntermediates -Eq $false) {
    $intermediateFiles | ForEach-Object {
      Write-Host "Cleaning up intermediate file $_"
      Remove-Item $_
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
    Write-Color -Text "Kraken ", "v$KRAKEN_BUILDING_VERSION_MAJOR.$KRAKEN_BUILDING_VERSION_MINOR ", "| ", "Pixar ", "v$PIXAR_BUILDING_VERSION ", "| ", "$KRAKEN_DEVELOPMENT_MILESTONE" -Color DarkMagenta, Cyan, Yellow, Green, Blue, Yellow, DarkMagenta
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

# Hop Into Kraken Root
Set-Alias krkn HopIntoRootDir

# Run Kraken with debugger console.
Set-Alias dbgkrkn RunAndDebugKraken

# Generate Kraken App Icons.
Set-Alias genicons CreateKrakenAppIcons

# Run Kraken
Set-Alias kraken_r RunDevelopmentReleaseKraken
Set-Alias kraken_d RunDevelopmentDebugKraken

Set-Alias python RunKrakenPythonOfficialRelease
Set-Alias python3 RunKrakenPythonOfficialRelease

# Run Kraken Python
if ($IsWindows) {
  Set-Alias python_r RunKrakenPythonRelease
  Set-Alias python_d RunKrakenPythonDebug
}

# Enter Kraken Server
Set-Alias wabiserver ConnectKraken

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