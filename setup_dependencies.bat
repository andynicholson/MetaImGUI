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

REM Download and setup Catch2 (for testing)
if not exist "external\catch2" (
    echo Downloading Catch2...
    cd external
    git clone https://github.com/catchorg/Catch2.git catch2
    cd catch2
    git checkout v3.4.0
    cd ..\..
    echo Catch2 downloaded successfully
) else (
    echo Catch2 already exists
)

REM Download and setup nlohmann/json (JSON library)
if not exist "external\json" (
    echo Downloading nlohmann/json...
    cd external
    git clone https://github.com/nlohmann/json.git
    cd json
    git checkout v3.11.3
    cd ..\..
    echo nlohmann/json downloaded successfully
) else (
    echo nlohmann/json already exists
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
