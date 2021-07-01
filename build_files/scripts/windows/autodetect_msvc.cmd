@REM Look into this detection precedence,
@REM as we want to prefer vs2022 over vs2019
@REM moving forward, if vs2022 is not available
@REM only then will we attempt vs2019 (which will
@REM not be officially supported moving forward !!)
@REM call "%~dp0\detect_msvc2019.cmd"
@REM if %ERRORLEVEL% EQU 0 goto DetectionComplete

call "%~dp0\detect_msvc2022.cmd"
if %ERRORLEVEL% EQU 0 goto DetectionComplete

echo Compiler Detection failed. Use verbose switch for more information.
exit /b 1

:DetectionComplete
echo Compiler Detection successful, detected VS%BUILD_VS_YEAR%
exit /b 0