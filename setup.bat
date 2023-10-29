powershell -Command "& {Expand-Archive data.zip data}"

@echo off
echo Enter the build directory. It should match 'Cmake --build <build_directory>'
set /p build_directory= Enter Build Directory Path (Relative to project root directory): 
echo Build Directory is: %build_directory%

powershell -Command "Invoke-WebRequest -Uri https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2212/dxc_2022_12_16.zip -OutFile dxc.zip"
powershell -Command "& {Expand-Archive dxc.zip external/dxc}"

xcopy external\dxc\bin\x64\* %build_directory%\bin\debug\
xcopy external\dxc\bin\x64\* %build_directory%\bin\release\

powershell -Command "Invoke-WebRequest -Uri https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.711.3-preview -OutFile agility.zip"
powershell -Command "& {Expand-Archive agility.zip external/agility-sdk}"

xcopy external\agility-sdk\build\native\bin\x64\* %build_directory%\bin\debug\D3D12\
xcopy external\agility-sdk\build\native\bin\x64\*  %build_directory%\bin\release\D3D12\
