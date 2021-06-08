@echo off

IF EXIST "C:\xyz\dev\build" (
    rmdir /s/q "C:\xyz\dev\build"
    mkdir "C:\xyz\dev\build"
)

IF EXIST "%WABI_ROOT%\deploy\modules" (
    rmdir /s/q "%WABI_ROOT%\deploy\modules"
    mkdir "%WABI_ROOT%\deploy\modules"
)

IF EXIST "%WABI_ROOT%\deploy\blender" (
    rmdir /s/q "%WABI_ROOT%\deploy\blender"
    mkdir "%WABI_ROOT%\deploy\blender"
)

IF EXIST %WABI_ROOT%\deploy\plugins (
    rmdir /s/q %WABI_ROOT%\deploy\plugins
    mkdir %WABI_ROOT%\deploy\plugins
)

IF EXIST %WABI_ROOT%\BtoA\python\AiZApi\lib (
    rmdir /s/q %WABI_ROOT%\BtoA\python\AiZApi\lib
)
IF EXIST %WABI_ROOT%\BtoA\python\AiZApi\build (
    rmdir /s/q %WABI_ROOT%\BtoA\python\AiZApi\build
)

IF EXIST %WABI_ROOT%\BtoA\python\AiZ\lib (
    rmdir /s/q %WABI_ROOT%\BtoA\python\AiZ\lib
)
IF EXIST %WABI_ROOT%\BtoA\python\AiZ\build (
    rmdir /s/q %WABI_ROOT%\BtoA\python\AiZ\build
)

IF EXIST %WABI_ROOT%\BtoA\python\AiDisplay\lib (
    rmdir /s/q %WABI_ROOT%\BtoA\python\AiDisplay\lib
)
IF EXIST %WABI_ROOT%\BtoA\python\AiDisplay\build (
    rmdir /s/q %WABI_ROOT%\BtoA\python\AiDisplay\build
)
IF EXIST %WABI_ROOT%\BtoA\python\AiGL\build (
    rmdir /s/q %WABI_ROOT%\BtoA\python\AiGL\build
)

IF EXIST "%BTOA_PATH%" (
    rmdir /s/q "%BTOA_PATH%"
    mkdir "%BTOA_PATH%"
)

echo.
echo All Clean.
echo.