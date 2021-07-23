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



function RunOfficialReleaseKraken {
  if($IsWindows) {
    & "$env:ProgramFiles\Wabi Animation\Kraken $KRAKEN_BUILDING_VERSION_MAJOR.$KRAKEN_BUILDING_VERSION_MINOR\kraken.exe" $args
  }
  if($IsMacOS) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
  }
  if($IsLinux) {
    Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
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

function DeleteConsoleLogs {
  clear
}

function KrakenPythonRelease {
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

function KrakenPythonDebug {
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

if($IsWindows) {
  Set-PoshPrompt -Theme "$env:USERPROFILE\dev\Kraken\build_files\build_environment\krakentheme.omp.json"
}
if($IsMacOS) {
  Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red
}
if($IsLinux) {
  Write-Color -Text "KrakenDeveloperProfile: Please configure paths for your platform." -Color Red  
}

Set-Alias makechaos RunOfficialReleaseKraken
Set-Alias krakenRunRel RunDevelopmentReleaseKraken
Set-Alias krakenRunDeb RunDevelopmentDebugKraken
Set-Alias xx DeleteConsoleLogs
Set-Alias python KrakenPythonRelease
Set-Alias python_d KrakenPythonDebug
Set-Alias wabiserver ConnectKraken
Set-Alias rr ReloadDeveloperProfile

ShowBanner