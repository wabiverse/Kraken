$KRAKEN_BUILDING_VERSION_MAJOR = 1
$KRAKEN_BUILDING_VERSION_MINOR = 50
$KRAKEN_DEVELOPMENT_MILESTONE = "Initial Release Sprint"
$PIXAR_BUILDING_VERSION = 21.08

$SHOW_KRAKEN_HUD = 1

$env:path ="$($env:path);."
$env:GIT_AUTHOR_EMAIL = "tyler@tylerfurby.com"

if(-not ($IsWindows)) {
  $env:PATH = "${env:PATH}:$Path"
}

$IsKrakenCreatorInDirectory = './source/creator'
$IsKrakenSourceInDirectory = './source/kraken/ChaosEngine'
$IsGitDirectory = './.git'

Import-Module -Name Terminal-Icons
Import-Module PSWriteColor
Import-Module oh-my-posh
Import-Module posh-git

function Get-Path {
  [CmdLetBinding()]
  Param()
  $PathFiles = @()
  $PathFiles += '/etc/paths'
  $PathFiles = Get-ChildItem -Path /private/etc/paths.d | Select-Object -Expand FullName
  $PathFiles | ForEach-Object {
    Get-Content -Path $PSItem | ForEach-Object {
      $_
    }
  }
  $Paths
}

function Add-Path {
  Param($Path)
  $env:PATH = "${env:PATH}:$Path"
}

function Update-Environment {
  [CmdLetBinding()]
  Param()
  $Paths = $env:PATH -split ':'
  Get-Path | ForEach-Object {
    if($PSItem -notin $Paths) {
      Write-Verbose "Adding $PSItem to Path"
      Add-Path -Path $PSItem
    }
  }
}

if($IsWindows) {
  $ChocolateyProfile = "$env:ChocolateyInstall\helpers\chocolateyProfile.psm1"
  if (Test-Path($ChocolateyProfile)) {
    Import-Module "$ChocolateyProfile"
  }
  # Only have to run this once.
  # & choco feature enable -n=allowGlobalConfirmation
}

# -------------------- This is called from command 'gg' -----

function WabiAnimationPreCommitHook 
{
  # hook.
  if(($Args[0] -eq 'commit') -or ($Args[0] -eq 'format')) {

    # clang format.
    # match hybrid CXX style between Google, Pixar, and Blender
    & git diff --cached --name-only | Where-Object {
      $_ -match '(\.cpp)|(\.json)'
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


function SetupEnv {
  cmd.exe /c "call `"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat`" && set > %temp%\vcvars.txt"
  
  Get-Content "$env:temp\vcvars.txt" | Foreach-Object {
    if ($_ -match "^(.*?)=(.*)$") {
      Set-Content "env:\$($matches[1])" $matches[2]
    }
  }
}


function RunMidlRT {
  & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\midlrt.exe" C:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.FOUNDATION.IDL /IC:\USERS\WABIF\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\CREATOR /I"C:\USERS\WABIF\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES" /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN /IC:\USERS\WABIF\DEV\KRAKEN /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\ANCHOR /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\EDITORS\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\KRAKLIB /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\KRAKERNEL /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\WM /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\LUXO /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\UNIVERSE /I"C:\USERS\WABIF\DEV\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\PYTHON\39\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\PTHREADS\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\PYTHON /IC:\VULKANSDK\1.2.189.2\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\TBB\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\PYTHON\39\INCLUDE /I"C:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\ALEMBIC\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE\IMATH /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\HDF5\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\DRACO\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\MATERIALX\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\ARNOLD\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\EMBREE\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OSL\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENIMAGEIO\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENSUBDIV\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\PTEX\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENCOLORIO\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENVDB\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDARNOLD\COMMON /IC:\USERS\WABIF\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDCYCLES\MIKKTSPACE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OIDN\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE\THIRD_PARTY\ATOMIC /I"C:\PROGRAM FILES\PIXAR\RENDERMANPROSERVER-24.1\INCLUDE" /metadata_dir "C:\PROGRAM FILES (X86)\WINDOWS KITS\10\REFERENCES\10.0.22000.0\WINDOWS.FOUNDATION.FOUNDATIONCONTRACT\4.0.0.0" /winrt /W1 /nologo /char signed /env x64 /out"C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\ChaosEngine" /winmd "C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\ChaosEngine\UNMERGED\KRAKEN.FOUNDATION.WINMD" /h "KRAKEN.FOUNDATION.H" /dlldata "NUL" /iid "KRAKEN.FOUNDATION_I.C" /proxy "KRAKEN.FOUNDATION_P.C" /tlb "KRAKEN.FOUNDATION.TLB" /client none /server none /enum_class /ns_prefix /target "NT60"  /nomidl @C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.midlrt.rsp  C:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.FOUNDATION.IDL
  & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\midlrt.exe" C:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.UIKIT.IDL /IC:\USERS\WABIF\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\CREATOR /I"C:\USERS\WABIF\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES" /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN /IC:\USERS\WABIF\DEV\KRAKEN /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\ANCHOR /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\EDITORS\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\KRAKLIB /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\KRAKERNEL /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\WM /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\LUXO /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\UNIVERSE /I"C:\USERS\WABIF\DEV\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\PYTHON\39\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\PTHREADS\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\PYTHON /IC:\VULKANSDK\1.2.189.2\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\TBB\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\PYTHON\39\INCLUDE /I"C:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\ALEMBIC\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE\IMATH /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\HDF5\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\DRACO\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\MATERIALX\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\ARNOLD\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\EMBREE\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OSL\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENIMAGEIO\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENSUBDIV\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\PTEX\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENCOLORIO\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENVDB\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDARNOLD\COMMON /IC:\USERS\WABIF\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDCYCLES\MIKKTSPACE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OIDN\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE\THIRD_PARTY\ATOMIC /I"C:\PROGRAM FILES\PIXAR\RENDERMANPROSERVER-24.1\INCLUDE" /metadata_dir "C:\PROGRAM FILES (X86)\WINDOWS KITS\10\REFERENCES\10.0.22000.0\WINDOWS.FOUNDATION.FOUNDATIONCONTRACT\4.0.0.0" /winrt /W1 /nologo /char signed /env x64 /out"C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\ChaosEngine" /winmd "C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\ChaosEngine\UNMERGED\KRAKEN.FOUNDATION.WINMD" /h "KRAKEN.FOUNDATION.H" /dlldata "NUL" /iid "KRAKEN.FOUNDATION_I.C" /proxy "KRAKEN.FOUNDATION_P.C" /tlb "KRAKEN.FOUNDATION.TLB" /client none /server none /enum_class /ns_prefix /target "NT60"  /nomidl @C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.midlrt.rsp  C:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\CHAOSENGINE\SRC\KRAKEN.UIKIT.IDL
  & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\midlrt.exe" 'C:\USERS\WABIF\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES\XAMLMETADATAPROVIDER.IDL' /IC:\USERS\WABIF\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\CREATOR /I"C:\USERS\WABIF\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES" /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN /IC:\USERS\WABIF\DEV\KRAKEN /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\ANCHOR /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\EDITORS\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\KRAKLIB /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\KRAKERNEL /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\WM /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\LUXO /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\UNIVERSE /I"C:\USERS\WABIF\DEV\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\PYTHON\39\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\PTHREADS\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\SOURCE\KRAKEN\PYTHON /IC:\VULKANSDK\1.2.189.2\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\TBB\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\PYTHON\39\INCLUDE /I"C:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\BOOST\INCLUDE\BOOST-1_78" /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\ALEMBIC\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE\IMATH /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\OPENEXR\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\HDF5\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\DRACO\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\MATERIALX\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\ARNOLD\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\EMBREE\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OSL\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENIMAGEIO\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENSUBDIV\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\PTEX\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENCOLORIO\INCLUDE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OPENVDB\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDARNOLD\COMMON /IC:\USERS\WABIF\DEV\KRAKEN\WABI\IMAGING\PLUGIN\HDCYCLES\MIKKTSPACE /IC:\USERS\WABIF\DEV\LIB\WIN64_VC17\OIDN\INCLUDE /IC:\USERS\WABIF\DEV\KRAKEN\..\LIB\WIN64_VC17\CYCLES\INCLUDE\THIRD_PARTY\ATOMIC /I"C:\PROGRAM FILES\PIXAR\RENDERMANPROSERVER-24.1\INCLUDE" /metadata_dir "C:\PROGRAM FILES (X86)\WINDOWS KITS\10\REFERENCES\10.0.22000.0\WINDOWS.FOUNDATION.FOUNDATIONCONTRACT\4.0.0.0" /winrt /W1 /nologo /char signed /env x64 /out"C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\ChaosEngine" /winmd "C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\ChaosEngine\UNMERGED\KRAKEN.FOUNDATION.WINMD" /h "KRAKEN.FOUNDATION.H" /dlldata "NUL" /iid "KRAKEN.FOUNDATION_I.C" /proxy "KRAKEN.FOUNDATION_P.C" /tlb "KRAKEN.FOUNDATION.TLB" /client none /server none /enum_class /ns_prefix /target "NT60"  /nomidl @C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.midlrt.rsp  'C:\USERS\WABIF\DEV\BUILD_KRAKEN_RELEASE\SOURCE\CREATOR\GENERATED FILES\XAMLMETADATAPROVIDER.IDL'
  & "C:\Users\wabif\dev\build_KRAKEN_Release\packages\Microsoft.Windows.CppWinRT.2.0.211028.7\bin\cppwinrt.exe" @C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.cppwinrt_ref.rsp -verbose
  & "C:\Users\wabif\dev\build_KRAKEN_Release\packages\Microsoft.Windows.CppWinRT.2.0.211028.7\bin\cppwinrt.exe" @C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.cppwinrt_comp.rsp -verbose
  # & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\mdmerge.exe" @C:\Users\wabif\dev\build_KRAKEN_Release\source\creator\kraken.dir\Release\kraken.vcxproj.mdmerge.rsp
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

function ReloadDeveloperProfile {
  . $PROFILE
}

function RunKrakenPythonOfficialRelease {
  if($IsWindows) {
    & "$env:ProgramFiles/Wabi Animation/Kraken $KRAKEN_BUILDING_VERSION_MAJOR.$KRAKEN_BUILDING_VERSION_MINOR/$KRAKEN_BUILDING_VERSION_MAJOR.$KRAKEN_BUILDING_VERSION_MINOR/python/bin/python.exe" $args
  }
  if($IsMacOS) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
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
  if (Test-Path -Path './.git') {
    $AUTHOR = git show --format="%an`n" -s
    $LATEST_REVISION = git show --summary --pretty=format:"%x07%h"
    $FOR_DATE = git show --summary --pretty=format:"%x07%ad"
    $DID_WHAT = git show --summary --pretty=format:"%x07%s"
    Write-Color -Text "Latest Revision: ", "$LATEST_REVISION ", "$FOR_DATE" -Color Red, Yellow, Green
    Write-Color -Text "      Work Done: ", "$DID_WHAT" -Color Blue, Cyan
    Write-Color -Text "    Authored by: ", "$AUTHOR" -Color Green, DarkMagenta
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

function DeleteConsoleLogs {
  clear
  ShowBanner
}

if($IsWindows) {
  Set-PoshPrompt -Theme "$env:USERPROFILE\dev\Kraken\build_files\build_environment\krakentheme.omp.json"
}
if($IsMacOS) {
  Set-PoshPrompt -Theme "~\dev\kraken\build_files\build_environment\krakentheme.omp.json"
}
if($IsLinux) {
  Set-PoshPrompt -Theme "~\dev\kraken\build_files\build_environment\krakentheme.omp.json"
}

# Run Kraken
Set-Alias kraken_r RunDevelopmentReleaseKraken
Set-Alias kraken_d RunDevelopmentDebugKraken

# Run Kraken Python
Set-Alias python RunKrakenPythonOfficialRelease
Set-Alias python_r RunKrakenPythonRelease
Set-Alias python_d RunKrakenPythonDebug

# Enter Kraken Server
Set-Alias wabiserver ConnectKraken

# Utility Convenience
Set-Alias xxx DeleteConsoleLogs
Set-Alias rr ReloadDeveloperProfile

Set-Alias gg WabiAnimationPreCommitHook

# Setup paths on macOS.
if($IsMacOS) {
  Update-Environment
}

# Print Pretty ASCII Logo Variant
ShowBanner