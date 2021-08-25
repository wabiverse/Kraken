if($IsWindows) {
#Requires -RunAsAdministrator
}


$KRAKEN_BUILDING_VERSION_MAJOR = 1
$KRAKEN_BUILDING_VERSION_MINOR = 50
$KRAKEN_DEVELOPMENT_MILESTONE = "Initial Release Sprint"
$PIXAR_BUILDING_VERSION = 21.08

$SHOW_KRAKEN_HUD = 1

$env:GIT_AUTHOR_EMAIL = "tyler@tylerfurby.com"


Set-Alias g git
Import-Module -Name Terminal-Icons
Import-Module PSWriteColor
Import-Module oh-my-posh
Import-Module posh-git

if($IsWindows) {
  $ChocolateyProfile = "$env:ChocolateyInstall\helpers\chocolateyProfile.psm1"
  if (Test-Path($ChocolateyProfile)) {
    Import-Module "$ChocolateyProfile"
  }
}

# -------------- This is called from .git/hooks/pre-commit -----

function WabiAnimationPreCommitHook {

  # Verify git email.
  if ($env:GIT_AUTHOR_EMAIL -notmatch '@(non\.)?tylerfurby\.com$') {
    Write-Warning "Your Git email address '$env:GIT_AUTHOR_EMAIL' is not configured correctly."
    Write-Warning "Use the command: 'git config --global user.email <tyler@tylerfurby.com>' to set it correctly."
    exit 1
  }

  # Add the changed files to git staging.
  Get-ChildItem (git diff --name-only) | Foreach-Object {

    $ammendFile = $_

    # Ensure we are only formatting the following extensions.
    $formatFileExt = [System.IO.Path]::GetExtension($ammendFile)
    if(($formatFileExt -eq '.cpp') -or ($formatFileExt -eq '.cc') -or ($formatFileExt -eq '.cpp') -or
        ($formatFileExt -eq '.cxx') -or ($formatFileExt -eq '.h') -or ($formatFileExt -eq '.hh') -or
        ($formatFileExt -eq '.hpp') -or ($formatFileExt -eq '.hxx') -or ($formatFileExt -eq '.m') -or
        ($formatFileExt -eq '.mm') -or ($formatFileExt -eq '.osl') -or ($formatFileExt -eq '.glsl')) {
      # Run clang format on each file.
      Write-Color -Text "Formatting: ", $ammendFile -Color Yellow, Green
      clang-format -style=file $ammendFile >$null 2>&1
    }

    if(-not($ammendFile -like '*vscode*')) {
      # Stage the files.
      git add $ammendFile
    }
  }
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
  $cert = @(Get-ChildItem -Path 'Cert:\CurrentUser\My\bfa7030dc5376b044f7b68d9c7a32f44a086ddbf')[0]
  $certBytes = $cert.Export([System.Security.Cryptography.X509Certificates.X509ContentType]::Pfx)
  [System.IO.File]::WriteAllBytes('C:\Users\tyler\dev\build_KRAKEN_Release\source\creator\wabianimation.kraken3d.pfx', $certBytes)
}

# CMake will call this command at build time in order to install the
# necessary NuGet packages Kraken needs to build. NuGet is no longer
# optional with the Windows 11 SDK as well as the .NET framework or
# consuming APIs with CppWinRT.
function InstallNugetPackages {
  & "C:\Program Files\devtools\nuget.exe" install ../build_KRAKEN_Release/packages.config -OutputDirectory ../build_KRAKEN_Release/packages
  & "C:\Program Files\devtools\nuget.exe" restore ../build_KRAKEN_Release/Kraken.sln
}

function ShowPrettyGitRevision {
  Write-Output " "
  $AUTHOR = git show --format="%an`n" -s
  $LATEST_REVISION = git show --summary --pretty=format:"%x07%h"
  $FOR_DATE = git show --summary --pretty=format:"%x07%ad"
  $DID_WHAT = git show --summary --pretty=format:"%x07%s"
  Write-Color -Text "Latest Revision: ", "$LATEST_REVISION ", "$FOR_DATE" -Color Red, Yellow, Green
  Write-Color -Text "      Work Done: ", "$DID_WHAT" -Color Blue, Cyan
  Write-Color -Text "    Authored by: ", "$AUTHOR" -Color Green, DarkMagenta
}

function ShowBanner {
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
  if($SHOW_KRAKEN_HUD) {
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
  Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
}
if($IsLinux) {
  Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red  
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
Set-Alias xx DeleteConsoleLogs
Set-Alias rr ReloadDeveloperProfile

# Print Pretty ASCII Logo Variant
ShowBanner