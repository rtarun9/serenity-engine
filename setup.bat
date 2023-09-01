@echo off
echo Enter the build directory.
echo The build directory should math 'Cmake --build <build_directory>'
set /p build_directory= Enter Build Directory Path (Relative to project root directory) : 
echo Build Directory is: %build_directory%

powershell -Command "Invoke-WebRequest -Uri https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2212/dxc_2022_12_16.zip -OutFile dxc.zip"
powershell -Command "& {Expand-Archive dxc.zip external/dxc}"

xcopy external\dxc\bin\x64\* %BuildDirectory%\bin\debug\
xcopy external\dxc\bin\x64\* %BuildDirectory%\bin\release\

xcopy external\dxc\bin\x64\* out\build\x64-Debug\bin\debug\
xcopy external\dxc\bin\x64\* out\build\x64-Release\bin\release\