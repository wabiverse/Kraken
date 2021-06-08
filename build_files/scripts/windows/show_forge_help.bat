@echo off
setlocal disableDelayedExpansion
set q=^"
echo(
echo(
call :c 01 "-----"&call :c 0F " THE WABI "&call :c 01 "------------------------------------------------ " /n
call :c 0E "* "&call :c 0A "arnold"&call :c 07 "       "&call :c 05 " BUILD ARNOLD RENDER ENGINE | 6.0.4.0 | 1.0 ALPHA " /n
call :c 0E "* "&call :c 0A "pixar"&call :c 07 "         BUILD PIXARS UNIVERSAL SCENE DESCRIPTION | 20.08" /n
call :c 0E "* "&call :c 0A "wabi"&call :c 07 "          BUILD THE PYTHON BACKEND & EXPORT, RELEASE BUILD"  /n
call :c 0E "* "&call :c 0A "run"&call :c 07 "           RUN BLENDER WITH ARNOLD, WITH TERMINAL DEBUGGING " /n
call :c 0E "* "&call :c 0A "tests"&call :c 07 "         BUILD WITH PROFILING, TESTS,  AND BENCHMARK WARS"  /n
call :c 0E "* "&call :c 0A "nuke"&call :c 07 "          NUKE EVERYTHING, EVERY FILE FROM BTOA AND ARNOLD"  /n
call :c 01 "---------------------------------------------------------------- " /n

if exist "%temp%\color.psm1" (
    powershell -command "&{set-executionpolicy remotesigned; Import-Module '%temp%\color.psm1'}"
    del "%temp%\color.psm1"
)

echo(

exit /b

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:c <color pair> <string> </n>
setlocal enabledelayedexpansion
set "colors=0-black;1-darkblue;2-darkgreen;3-darkcyan;4-darkred;5-darkmagenta;6-darkyellow;7-gray;8-darkgray;9-blue;a-green;b-cyan;c-red;d-magenta;e-yellow;f-white"
set "p=%~1"
set "bg=!colors:*%p:~0,1%-=!"
set bg=%bg:;=&rem.%
set "fg=!colors:*%p:~-1%-=!"
set fg=%fg:;=&rem.%

if not "%~3"=="/n" set "br=-nonewline"
set "str=%~2" & set "str=!str:'=''!"

>>"%temp%\color.psm1" echo write-host '!str!' -foregroundcolor '%fg%' -backgroundcolor '%bg%' %br%
endlocal