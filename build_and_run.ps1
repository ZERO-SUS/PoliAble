$env:Path = "E:\Visual_studio\VC\Tools\MSVC\14.51.36231\bin\Hostx64\x64;E:\JUNK\Tools\Ninja;$env:Path"
$env:INCLUDE = "E:\Visual_studio\VC\Tools\MSVC\14.51.36231\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared"
$env:LIB = "E:\Visual_studio\VC\Tools\MSVC\14.51.36231\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64"
$env:LIBPATH = "E:\Visual_studio\VC\Tools\MSVC\14.51.36231\lib\x64"

Set-Location "C:\Users\Admin\source\repos\PoliAble\build"

Write-Host "Running CMake configuration..."
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DQT_NO_PACKAGE_VERSION_CHECK=TRUE -DQT_NO_PACKAGE_VERSION_INCOMPATIBLE_WARNING=TRUE -DCMAKE_C_COMPILER="E:/Visual_studio/VC/Tools/MSVC/14.51.36231/bin/Hostx64/x64/cl.exe" -DCMAKE_CXX_COMPILER="E:/Visual_studio/VC/Tools/MSVC/14.51.36231/bin/Hostx64/x64/cl.exe" ..

if ($LASTEXITCODE -eq 0) {
    Write-Host "Running Ninja build..."
    ninja -v

    if ($LASTEXITCODE -eq 0) {
        Write-Host "Build succeeded! Launching PoliAble..."
        & ".\PoliAble.exe"
    } else {
        Write-Host "Build failed with exit code: $LASTEXITCODE"
    }
} else {
    Write-Host "CMake configuration failed with exit code: $LASTEXITCODE"
}
