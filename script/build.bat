@echo off
cd /d %~dp0
echo off
cd ..
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
cd build
Ninja
