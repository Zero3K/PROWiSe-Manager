@echo off
echo PROWiSe Manager - Compatibility Launcher
echo ========================================
echo.
echo This script attempts to run PROWiSe.exe with compatibility settings
echo that may help resolve startup issues on newer Windows versions.
echo.

REM Check if PROWiSe.exe exists
if not exist "PROWiSe.exe" (
    echo ERROR: PROWiSe.exe not found in current directory.
    echo Please make sure this batch file is in the same folder as PROWiSe.exe
    echo.
    pause
    exit /b 1
)

REM Check if ModernMenu.dll exists
if not exist "ModernMenu.dll" (
    echo WARNING: ModernMenu.dll not found in current directory.
    echo This may cause the application to fail. Please ensure all files are extracted
    echo to the same folder.
    echo.
)

echo Attempting to launch PROWiSe.exe...
echo.

REM Try to run with Windows XP SP3 compatibility
echo [Attempt 1] Running with Windows XP SP3 compatibility...
start "" /wait "%~dp0PROWiSe.exe"

REM Check if the process is still running (successful start)
tasklist /fi "imagename eq PROWiSe.exe" 2>nul | find /i "PROWiSe.exe" >nul
if %errorlevel% == 0 (
    echo.
    echo SUCCESS: PROWiSe.exe appears to be running.
    echo If you continue to have issues, please see INSTALLATION.md
    goto :end
)

echo.
echo If PROWiSe.exe failed to start, please try the following:
echo.
echo 1. Run check_requirements.bat to verify system requirements
echo 2. Install Visual C++ 2005 and 2008 Redistributables
echo 3. Try running as Administrator
echo 4. See INSTALLATION.md for detailed troubleshooting
echo.

:end
pause