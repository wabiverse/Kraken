@echo off

cd "C:\xyz\dev\build"

cmake %WABI_ROOT% ^
-D ARNOLD_BUILD_PYTHON=ON ^
-D ARNOLD_HOME="%Arnold_HOME%" ^
-D ARNOLD_BUILD_PYTHON=ON ^
-D CMAKE_BUILD_TYPE=RELEASE ^
-G Ninja

ninja -j36 all
ninja -j36 install