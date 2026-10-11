@echo off
call "E:\Visual_studio\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
"E:\Visual_studio\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe" -C out/build/x64-debug %*
