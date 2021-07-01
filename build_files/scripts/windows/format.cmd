if EXIST %COVAH_DIR%\..\lib\win64_%BUILD_VS_LIBDIRPOST%\llvm\bin\clang-format.exe (
    set CF_PATH=..\lib\win64_%BUILD_VS_LIBDIRPOST%\llvm\bin
    goto detect_done
)

echo clang-format not found
exit /b 1

:detect_done
echo found clang-format in %CF_PATH%

if EXIST %PYTHON% (
    set PYTHON=%COVAH_DIR%\..\lib\win64_%BUILD_VS_LIBDIRPOST%\python\39\bin\python.exe
    goto detect_python_done
)

echo python not found in lib folder
exit /b 1

:detect_python_done
echo found python (%PYTHON%)

set FORMAT_PATHS=%COVAH_DIR%\source\tools\utils_maintenance\clang_format_paths.py

REM The formatting script expects clang-format to be in the current PATH.
set PATH=%CF_PATH%;%PATH%

REM Use -B to avoid writing __pycache__ in lib directory and causing update conflicts.
%PYTHON% -B %FORMAT_PATHS% %FORMAT_ARGS%

:EOF
