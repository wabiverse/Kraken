set COVAH_DIR_NOSPACES=%COVAH_DIR: =%

if not "%COVAH_DIR%"=="%COVAH_DIR_NOSPACES%" (
	echo There are spaces detected in the build path "%COVAH_DIR%", this is currently not supported, exiting....
	exit /b 1
)