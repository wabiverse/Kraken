set BUILD_DIR=%COVAH_DIR%..\build_COVAH
set BUILD_TYPE=Release
:argv_loop
if NOT "%1" == "" (

    if "%1" == "help" (
        set SHOW_HELP=1
        goto EOF
    )
	if "%1" == "debug" (
		set BUILD_TYPE=Debug
		set CMAKE_BUILD_TYPE_INIT=Debug
	REM Build Configurations
	) else if "%1" == "builddir" (
		set BUILD_DIR_OVERRRIDE=%COVAH_DIR%..\%2
		shift /1
	) else if "%1" == "with_tests" (
		set TESTS_CMAKE_ARGS=%TESTS_CMAKE_ARGS%
	) else if "%1" == "with_opengl_tests" (
		set TESTS_CMAKE_ARGS=%TESTS_CMAKE_ARGS%
	) else if "%1" == "full" (
		set TARGET=Full
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% ^
		    -C"%COVAH_DIR%\build_files\cmake\config\covah_full.cmake"
	) else if "%1" == "lite" (
		set TARGET=Lite
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% -C"%COVAH_DIR%\build_files\cmake\config\covah_lite.cmake"
	) else if "%1" == "arnold" (
		set TARGET=Arnold
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% -C"%COVAH_DIR%\build_files\cmake\config\arnold_standalone.cmake"
	) else if "%1" == "headless" (
		set TARGET=Headless
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% -C"%COVAH_DIR%\build_files\cmake\config\covah_headless.cmake"
	) else if "%1" == "wabi" (
		set TARGET=Pxr
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% -C"%COVAH_DIR%\build_files\cmake\config\wabi_module.cmake"
	) else if "%1" == "clang" (
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS%
		set WITH_CLANG=1
 	) else if "%1" == "release" (
		set TARGET=Release
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% -C"%COVAH_DIR%\build_files\cmake\config\covah_release.cmake"
	) else if "%1" == "developer" (
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% -C"%COVAH_DIR%\build_files\cmake\config\covah_developer.cmake"
	) else if "%1" == "asan" (
		set WITH_ASAN=1
	) else if "%1" == "x86" (
		echo Error: 32 bit builds of COVAH are no longer supported.
		goto ERR
	) else if "%1" == "x64" (
		set BUILD_ARCH=x64
	) else if "%1" == "2017" (
		set BUILD_VS_YEAR=2017
	) else if "%1" == "2017pre" (
		set BUILD_VS_YEAR=2017
		set VSWHERE_ARGS=-prerelease
	) else if "%1" == "2017b" (
		set BUILD_VS_YEAR=2017
		set VSWHERE_ARGS=-products Microsoft.VisualStudio.Product.BuildTools
	) else if "%1" == "2019" (
		set BUILD_VS_YEAR=2019
	) else if "%1" == "2019pre" (
		set BUILD_VS_YEAR=2019
		set VSWHERE_ARGS=-prerelease
	) else if "%1" == "2019b" (
		set BUILD_VS_YEAR=2019
		set VSWHERE_ARGS=-products Microsoft.VisualStudio.Product.BuildTools
	) else if "%1" == "packagename" (
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% -DCPACK_OVERRIDE_PACKAGENAME="%2"
		shift /1
	) else if "%1" == "nobuild" (
		set NOBUILD=1
	) else if "%1" == "nobuildinfo" (
		set BUILD_CMAKE_ARGS=%BUILD_CMAKE_ARGS% -DWITH_BUILDINFO=Off
	) else if "%1" == "pydebug" (
		set WITH_PYDEBUG=1
	) else if "%1" == "showhash" (
		SET BUILD_SHOW_HASHES=1
	REM Non-Build Commands
	) else if "%1" == "arnold" (
		SET UPDATE_ARNOLD_PYTHON=1
	) else if "%1" == "update" (
		SET BUILD_UPDATE=1
		set BUILD_UPDATE_ARGS=
	) else if "%1" == "code_update" (
		SET BUILD_UPDATE=1
		set BUILD_UPDATE_ARGS="--no-libraries"
	) else if "%1" == "ninja" (
		SET BUILD_WITH_NINJA=1
	) else if "%1" == "sccache" (
		SET BUILD_WITH_SCCACHE=1
	) else if "%1" == "clean" (
		set MUST_CLEAN=1
	) else if "%1" == "verbose" (
		set VERBOSE=1
	) else if "%1" == "test" (
		set TEST=1
		set NOBUILD=1
	) else if "%1" == "format" (
		set FORMAT=1
		set FORMAT_ARGS=%2 %3 %4 %5 %6 %7 %8 %9
		goto EOF
	) else if "%1" == "docs" (
		set DOCS=1
		set DOCS_ARGS=%2
		goto EOF
	) else if "%1" == "buildenv" (
		set BUILD_ENVIRONMENT=1
		set BUILD_ENVIRONMENT_ARGS=%2 %3 %4 %5 %6 %7 %8 %9
		goto EOF
	) else if "%1" == "auto_cmake" (
		set AUTOGENERATE_CMAKE=1
		set AUTOGENERATE_CMAKE_ARG=%2
		goto EOF
  ) else (
        echo Command "%1" unknown, aborting...
        echo use --help for build options.
        goto ERR
    )
    shift /1
	goto argv_loop
)
:EOF
exit /b 0
:ERR
exit /b 1
