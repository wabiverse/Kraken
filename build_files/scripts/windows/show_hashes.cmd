if "%GIT%" == "" (
	echo Git not found, cannot show hashes.
	goto EOF
)
cd "%KRAKEN_DIR%"
for /f "delims=" %%i in ('"%GIT%" rev-parse HEAD') do echo Branch_hash=%%i
cd "%KRAKEN_DIR%"
:EOF