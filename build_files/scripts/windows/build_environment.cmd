if EXIST %PYTHON% (
    set PYTHON=%BUILD_VS_LIBDIR%\python\39\bin\python.exe
    goto detect_python_done
) else (
    if NOT exist "%BUILD_VS_LIBDIR%\python" mkdir "%BUILD_VS_LIBDIR%\python"
    cd "%BUILD_VS_LIBDIR%\python"
    call svn checkout https://svn.blender.org/svnroot/bf-blender/trunk/lib/win64_vc15/python/39/
    set PYTHON=%BUILD_VS_LIBDIR%\python\39\bin\python.exe
    @REM Install required pip dependencies for our
    @REM dependency installation script. Really,
    @REM just rarfile since github .zip's like to
    @REM secretly disguise themselves as .rar's.
    @REM ¯\_(ツ)_/¯ Upgrade pip while we're at it.
    %PYTHON% -B -m pip install --upgrade pip
    %PYTHON% -B -m pip install rarfile
    goto detect_python_done
)

:detect_python_done
set INSTALL_DEPS_PY=%COVAH_DIR%\build_files\build_environment\install_deps.py

if NOT "%VS2022_NOT_OFFICIALLY_RELEASED%" == "" (
    echo.
    echo MSVC 2022 is not officially released.
    echo Until then, you will need to compile
    echo The Master Branch of Cmake for MSVC
    echo 2022 support.
    echo.
)

if NOT "%BUILD_ENVIRONMENT_ARGS%" == "" (
    @REM Preserve arguments to auto-dependency
    @REM builder if user passes their own Args in...
    %PYTHON% -B %INSTALL_DEPS_PY% %BUILD_ENVIRONMENT_ARGS%
    goto EOF
) else (
    @REM Otherwise, go ahead and grab some popcorn.
    @REM This is going to take a minute...
    %PYTHON% -B %INSTALL_DEPS_PY% --build all
    goto EOF
)

:EOF
