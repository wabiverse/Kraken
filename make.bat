@echo off

setlocal EnableDelayedExpansion
setlocal ENABLEEXTENSIONS
set KRAKEN_DIR=%~dp0

call "%KRAKEN_DIR%\build_files\scripts\windows\reset_args_and_stuff.cmd"

call "%KRAKEN_DIR%\build_files\scripts\windows\check_args.cmd" %*
if errorlevel 1 goto EOF

call "%KRAKEN_DIR%\build_files\scripts\windows\find_dependencies.cmd"
if errorlevel 1 goto EOF

if "%BUILD_SHOW_HASHES%" == "1" (
	call "%KRAKEN_DIR%\build_files\scripts\windows\show_hashes.cmd"
	goto EOF
)

if "%SHOW_HELP%" == "1" (
  call "%KRAKEN_DIR%\build_files\scripts\windows\show_help.cmd"
  goto EOF
)

if "%DOCS%" == "1" (
	call "%KRAKEN_DIR%\build_files\scripts\windows\build_docs.cmd"
	goto EOF
)

if "%AUTOGENERATE_CMAKE%" == "1" (
	call "%KRAKEN_DIR%\build_files\scripts\windows\cmake_autogen.cmd"
	goto EOF
)

call "%KRAKEN_DIR%\build_files\scripts\windows\detect_architecture.cmd"
if errorlevel 1 goto EOF

if "%BUILD_VS_YEAR%" == "" (
	call "%KRAKEN_DIR%\build_files\scripts\windows\autodetect_msvc.cmd"
	if errorlevel 1 (
		echo Visual Studio not found ^(try with the 'verbose' switch for more information^)
		goto EOF
	)
) else (
	call "%KRAKEN_DIR%\build_files\scripts\windows\detect_msvc%BUILD_VS_YEAR%.cmd"
	if errorlevel 1 (
		echo Visual Studio %BUILD_VS_YEAR% not found ^(try with the 'verbose' switch for more information^)
		goto EOF
	)
)

if "%FORMAT%" == "1" (
	call "%KRAKEN_DIR%\build_files\scripts\windows\check_libraries.cmd"
	call "%KRAKEN_DIR%\build_files\scripts\windows\format.cmd"
	goto EOF
)

if "%PRECOMMIT_HOOK%" == "1" (
	call pwsh.exe -ExecutionPolicy RemoteSigned -File "%KRAKEN_DIR%\.git\hooks\pre-commit-hook.ps1"
	goto EOF
)

if "%BUILD_ENVIRONMENT%" == "1" (
	call "%KRAKEN_DIR%\build_files\scripts\windows\check_libraries.cmd"
	call "%KRAKEN_DIR%\build_files\scripts\windows\build_environment.cmd"
	goto EOF
)

if "%BUILD_UPDATE%" == "1" (
	call "%KRAKEN_DIR%\build_files\scripts\windows\check_libraries.cmd"
	if errorlevel 1 goto EOF

	call "%KRAKEN_DIR%\build_files\scripts\windows\update_sources.cmd"
	goto EOF
)

call "%KRAKEN_DIR%build_files\scripts\windows\set_build_dir.cmd"

echo Building Kraken with VS%\BUILD_VS_YEAR% for %BUILD_ARCH% in %BUILD_DIR%

call "%KRAKEN_DIR%\build_files\scripts\windows\check_libraries.cmd"
if errorlevel 1 goto EOF

if "%TEST%" == "1" (
	call "%KRAKEN_DIR%\build_files\scripts\windows\test.cmd"
	goto EOF
)

if "%BUILD_WITH_NINJA%" == "" (
	call "%KRAKEN_DIR%\build_files\scripts\windows\configure_msbuild.cmd"
	if errorlevel 1 goto EOF

	call "%KRAKEN_DIR%\build_files\scripts\windows\build_msbuild.cmd"
	if errorlevel 1 goto EOF
) else (
	call "%KRAKEN_DIR%\build_files\scripts\windows\configure_ninja.cmd"
	if errorlevel 1 goto EOF

	call "%KRAKEN_DIR%\build_files\scripts\windows\build_ninja.cmd"
	if errorlevel 1 goto EOF
)

:EOF