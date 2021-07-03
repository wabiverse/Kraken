if not "%AUTOGENERATE_CMAKE_ARG%" == "" (
    goto proceed
)

echo.
echo No path to CMakeLists file was given
echo.
exit /b 1

:proceed
set AUTOGENERATE_CMAKE_SCRIPT=%KRAKEN_DIR%\build_files\utils\cmake_generator.py

echo.
echo %AUTOGENERATE_CMAKE_SCRIPT%
echo.

%PYTHON% %AUTOGENERATE_CMAKE_SCRIPT% %1 %AUTOGENERATE_CMAKE_ARG%