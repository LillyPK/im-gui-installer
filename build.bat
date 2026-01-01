@echo off
echo ============================================
echo Building Stylized Installer
echo ============================================
echo.

REM Create build directory if it doesn't exist
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

REM Navigate to build directory
cd build

REM Run CMake to generate project files
echo.
echo Generating project files with CMake...
cmake ..
if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo Make sure CMake is installed and in your PATH.
    pause
    exit /b %errorlevel%
)

REM Build the project in Release mode
echo.
echo Building project in Release mode...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b %errorlevel%
)

echo.
echo ============================================
echo Build completed successfully!
echo ============================================
echo.

REM Find and display the executable location
echo Looking for executable...
if exist "bin\Release\StylizedInstaller.exe" (
    echo.
    echo EXE found at: build\bin\Release\StylizedInstaller.exe
    echo.
    echo Do you want to run the installer now? (Y/N)
    set /p run_now=
    if /i "%run_now%"=="Y" (
        start bin\Release\StylizedInstaller.exe
    )
) else if exist "Release\StylizedInstaller.exe" (
    echo.
    echo EXE found at: build\Release\StylizedInstaller.exe
    echo.
    echo Do you want to run the installer now? (Y/N)
    set /p run_now=
    if /i "%run_now%"=="Y" (
        start Release\StylizedInstaller.exe
    )
) else (
    echo.
    echo WARNING: Could not find StylizedInstaller.exe
    echo Searching for any .exe files...
    dir /s /b *.exe
)

echo.
pause
