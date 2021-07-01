@echo off
if not defined DevEnvDir (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" amd64
)

set PYTHONPATH=C:\Users\tyler\.conda\envs\pixar\Lib\site-packages
conda activate pixar