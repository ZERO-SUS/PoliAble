@echo off
call "E:\Visual_studio\VC\Auxiliary\Build\vcvars64.bat"
cd /d "%~dp0build"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
