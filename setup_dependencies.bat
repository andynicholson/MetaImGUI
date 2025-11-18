@echo off
echo Setting up MetaImGUI dependencies...

REM Create external directory
if not exist "external" mkdir external

REM Download and setup ImGui
if not exist "external\imgui" (
    echo Downloading ImGui...
    cd external
    git clone https://github.com/ocornut/imgui.git
    cd imgui
    git checkout v1.92.4
    cd ..\..
    echo ImGui downloaded successfully
) else (
    echo ImGui already exists
)

REM Create build directory
if not exist "build" mkdir build

echo Dependencies setup complete!
echo To build:
echo   cd build
echo   cmake ..
echo   cmake --build . --config Release
pause 