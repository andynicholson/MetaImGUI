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

REM Install pre-commit hook for automatic code formatting
echo.
echo Installing pre-commit hook...
if exist ".pre-commit-hook.sh" (
    copy .pre-commit-hook.sh .git\hooks\pre-commit >nul
    echo Pre-commit hook installed (automatic code formatting before commits)
) else (
    echo Warning: Pre-commit hook script not found
)

echo.
echo Dependencies setup complete!
echo To build:
echo   cd build
echo   cmake ..
echo   cmake --build . --config Release
pause
