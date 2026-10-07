@echo off
call "E:\Visual_studio\VC\Auxiliary\Build\vcvars64.bat"
cd /d "C:\Users\Admin\source\repos\PoliAble\build"
ninja -v
if %ERRORLEVEL% EQU 0 (
    echo Build succeeded!
    PoliAble.exe
) else (
    echo Build failed with error code %ERRORLEVEL%
)
