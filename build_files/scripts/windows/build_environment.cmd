if EXIST %PYTHON% (
    set PYTHON=%COVAH_DIR%\..\lib\win64_%BUILD_VS_LIBDIRPOST%\python\39\bin\python.exe
    goto detect_python_done
)

echo python not found in lib folder
exit /b 1

:detect_python_done
set INSTALL_DEPS_PY=%COVAH_DIR%\build_files\build_environment\install_deps.py

REM Use -B to avoid writing __pycache__ in lib directory and causing update conflicts.
%PYTHON% -B %INSTALL_DEPS_PY% %BUILD_ENVIRONMENT_ARGS%

:EOF
