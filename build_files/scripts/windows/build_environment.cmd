if EXIST %PYTHON% (
    set PYTHON=%BUILD_VS_LIBDIR%\python\39\bin\python.exe
    goto detect_python_done
) else (
    if NOT exist "%BUILD_VS_LIBDIR%\python" mkdir "%BUILD_VS_LIBDIR%\python"
    cd "%BUILD_VS_LIBDIR%\python"
    call svn checkout https://svn.blender.org/svnroot/bf-blender/trunk/lib/win64_vc15/python/39/
    set PYTHON=%BUILD_VS_LIBDIR%\python\39\bin\python.exe
    goto detect_python_done
)

:detect_python_done
set INSTALL_DEPS_PY=%COVAH_DIR%\build_files\build_environment\install_deps.py

REM Use -B to avoid writing __pycache__ in lib directory and causing update conflicts.
%PYTHON% -B %INSTALL_DEPS_PY% %BUILD_ENVIRONMENT_ARGS%

:EOF
