echo.
echo Convenience targets
echo - release ^(identical to the official kraken.org builds^)
echo - headless
echo - wabipy ^(Wabi Python, All things 3D, wrapped in a single Python Module^)
echo.
echo Utilities ^(not associated with building^)
echo - clean ^(Target must be set^)
echo - update ^(Update GIT source^)
echo - deps ^(Build and install all required dependencies to build Kraken ^(and many other things^) from scratch^)
echo - nobuild ^(only generate project files^)
echo - showhash ^(Show git hashes of source tree^)
echo - test ^(Run automated tests with ctest^)
echo - format [path] ^(Format the source using clang-format, path is optional, requires python 3.x to be available^)
echo.
echo Configuration options
echo - verbose ^(enable diagnostic output during configuration^)
echo - developer ^(enable faster builds, error checking and tests, recommended for developers^)
echo - with_tests ^(enable building unit tests^)
echo - nobuildinfo ^(disable buildinfo^)
echo - debug ^(Build an unoptimized debuggable build^)
echo - packagename [newname] ^(override default cpack package name^)
echo - builddir [newdir] ^(override default build folder^)
echo - 2019 ^(build with visual studio 2019^)
echo - 2019pre ^(build with visual studio 2019 pre-release^)
echo - 2019b ^(build with visual studio 2019 Build Tools^)
echo - 2022 ^(build with visual studio 2022^)
echo - 2022pre ^(build with visual studio 2022 pre-release^)
echo - 2022b ^(build with visual studio 2022 Build Tools^)

echo.
echo Experimental options
echo - clang ^(enable building with clang^)
echo - asan ^(enable asan when building with clang^)
echo - ninja ^(enable building with ninja instead of msbuild^)
echo.