@echo off
echo Building MetaImGUI...

REM Create build directory if it doesn't exist
if not exist "build" mkdir build

REM Configure and build
cd build
cmake ..
cmake --build . --config Release

if %ERRORLEVEL% EQU 0 (
    echo Build successful! Run with: build\Release\MetaImGUI.exe
) else (
    echo Build failed! Check the error messages above.
    pause
    exit /b 1
) 