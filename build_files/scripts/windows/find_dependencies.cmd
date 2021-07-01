for %%X in (svn.exe) do (set SVN=%%~$PATH:X)
for %%X in (cmake.exe) do (set CMAKE=%%~$PATH:X)
for %%X in (ctest.exe) do (set CTEST=%%~$PATH:X)
for %%X in (git.exe) do (set GIT=%%~$PATH:X)
set PYTHON=%COVAH_DIR%\..\lib\win64_%BUILD_VS_LIBDIRPOST%\python\39\bin\python.exe
if NOT "%verbose%" == "" (
	echo svn    : "%SVN%"
	echo cmake  : "%CMAKE%"
	echo ctest  : "%CTEST%"
	echo git    : "%GIT%"
	echo python : "%PYTHON%"
)
if "%CMAKE%" == "" (
	echo Cmake not found in path, required for building, exiting...
	exit /b 1
)