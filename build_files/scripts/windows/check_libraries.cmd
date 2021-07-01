if "%BUILD_VS_YEAR%"=="2019" set BUILD_VS_LIBDIRPOST=vc16
if "%BUILD_VS_YEAR%"=="2022" set BUILD_VS_LIBDIRPOST=vc17

set BUILD_VS_SVNDIR=win64_%BUILD_VS_LIBDIRPOST%
set BUILD_VS_LIBDIR="%COVAH_DIR%..\lib\%BUILD_VS_SVNDIR%"

if NOT "%verbose%" == "" (
	echo Library Directory = "%BUILD_VS_LIBDIR%"
)

if NOT EXIST %BUILD_VS_LIBDIR% (
	echo.
	echo Error: Required libraries not found at "%BUILD_VS_LIBDIR%"
	echo This is needed for building, aborting!
	echo.
	exit /b 1
)