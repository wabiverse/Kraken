if EXIST %BUILD_VS_LIBDIR%\llvm\bin\clang-format.exe (
    set CF_PATH=..\lib\win64_%BUILD_VS_LIBDIRPOST%\llvm\bin
    goto detect_done
)

echo clang-format not found
exit /b 1

:detect_done
echo found clang-format in %CF_PATH%

set PYTHON=%BUILD_VS_LIBDIR%\python\39\bin\python.exe

set FORMAT_PATHS=%COVAH_DIR%\source\tools\utils_maintenance\clang_format_paths.py

REM The formatting script expects clang-format to be in the current PATH.
set PATH=%CF_PATH%;%PATH%

REM Use -B to avoid writing __pycache__ in lib directory and causing update conflicts.
%PYTHON% -B %FORMAT_PATHS% %FORMAT_ARGS%

:EOF
