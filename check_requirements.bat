@echo off
echo PROWiSe Manager - System Requirements Checker
echo =============================================
echo.

REM Check Windows version
ver | findstr /i "6\." >nul
if %errorlevel% == 0 (
    echo [OK] Windows Vista/7 detected
    goto :checkruntime
)

ver | findstr /i "10\." >nul
if %errorlevel% == 0 (
    echo [OK] Windows 10/11 detected
    goto :checkruntime
)

ver | findstr /i "5\." >nul
if %errorlevel% == 0 (
    echo [OK] Windows XP/2003 detected
    goto :checkruntime
)

echo [WARNING] Could not determine Windows version
echo.

:checkruntime
echo Checking for required runtime libraries...
echo.

REM Check for Visual C++ 2005 Runtime (common requirement for apps built with older tools)
if exist "%SystemRoot%\System32\msvcr80.dll" (
    echo [OK] Visual C++ 2005 Runtime found
) else (
    echo [MISSING] Visual C++ 2005 Runtime (msvcr80.dll)
    echo   Download from: https://www.microsoft.com/en-us/download/details.aspx?id=26347
)

REM Check for Visual C++ 2008 Runtime
if exist "%SystemRoot%\System32\msvcr90.dll" (
    echo [OK] Visual C++ 2008 Runtime found
) else (
    echo [MISSING] Visual C++ 2008 Runtime (msvcr90.dll)
    echo   Download from: https://www.microsoft.com/en-us/download/details.aspx?id=29
)

REM Check for Visual C++ 2010 Runtime
if exist "%SystemRoot%\System32\msvcr100.dll" (
    echo [OK] Visual C++ 2010 Runtime found
) else (
    echo [INFO] Visual C++ 2010 Runtime not found (may not be required)
)

echo.
echo Checking for Common Controls...
if exist "%SystemRoot%\System32\comctl32.dll" (
    echo [OK] Common Controls found
) else (
    echo [ERROR] Common Controls missing - this is unusual
)

echo.
echo =============================================
echo If PROWiSe.exe still fails to start after installing missing runtimes,
echo try running it as Administrator or in Windows XP compatibility mode.
echo.
echo To run in compatibility mode:
echo 1. Right-click on PROWiSe.exe
echo 2. Select Properties
echo 3. Go to Compatibility tab
echo 4. Check "Run this program in compatibility mode for:"
echo 5. Select "Windows XP (Service Pack 3)"
echo 6. Check "Run this program as an administrator"
echo 7. Click OK
echo.
pause