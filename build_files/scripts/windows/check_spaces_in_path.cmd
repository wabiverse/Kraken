set KRAKEN_DIR_NOSPACES=%KRAKEN_DIR: =%

if not "%KRAKEN_DIR%"=="%KRAKEN_DIR_NOSPACES%" (
	echo There are spaces detected in the build path "%KRAKEN_DIR%", this is currently not supported, exiting....
	exit /b 1
)