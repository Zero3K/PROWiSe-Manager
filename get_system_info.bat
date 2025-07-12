@echo off
title PROWiSe Manager - System Information
echo PROWiSe Manager - System Information
echo ====================================
echo.
echo Gathering system information to help diagnose startup issues...
echo.

echo WINDOWS VERSION:
ver
echo.

echo SYSTEM ARCHITECTURE:
echo %PROCESSOR_ARCHITECTURE%
if defined PROCESSOR_ARCHITEW6432 echo %PROCESSOR_ARCHITEW6432% (WOW64)
echo.

echo CURRENT USER:
echo %USERNAME%
echo.

echo SYSTEM ROOT:
echo %SystemRoot%
echo.

echo PROGRAM FILES:
echo %ProgramFiles%
if defined ProgramFiles(x86) echo %ProgramFiles(x86)%
echo.

echo CHECKING CRITICAL SYSTEM FILES:
echo.

if exist "%SystemRoot%\System32\kernel32.dll" (
    echo [OK] kernel32.dll found
) else (
    echo [ERROR] kernel32.dll missing - serious system issue
)

if exist "%SystemRoot%\System32\user32.dll" (
    echo [OK] user32.dll found
) else (
    echo [ERROR] user32.dll missing - serious system issue
)

if exist "%SystemRoot%\System32\comctl32.dll" (
    echo [OK] comctl32.dll found
) else (
    echo [ERROR] comctl32.dll missing
)

echo.
echo CHECKING VISUAL C++ RUNTIMES:
echo.

if exist "%SystemRoot%\System32\msvcr71.dll" (
    echo [OK] Visual C++ 2003 Runtime found
) else (
    echo [INFO] Visual C++ 2003 Runtime not found
)

if exist "%SystemRoot%\System32\msvcr80.dll" (
    echo [OK] Visual C++ 2005 Runtime found
) else (
    echo [MISSING] Visual C++ 2005 Runtime - INSTALL THIS
    echo          Download: https://www.microsoft.com/en-us/download/details.aspx?id=26347
)

if exist "%SystemRoot%\System32\msvcr90.dll" (
    echo [OK] Visual C++ 2008 Runtime found
) else (
    echo [MISSING] Visual C++ 2008 Runtime - INSTALL THIS
    echo          Download: https://www.microsoft.com/en-us/download/details.aspx?id=29
)

if exist "%SystemRoot%\System32\msvcr100.dll" (
    echo [OK] Visual C++ 2010 Runtime found
) else (
    echo [INFO] Visual C++ 2010 Runtime not found (may not be needed)
)

echo.
echo CHECKING LOCAL FILES:
echo.

if exist "PROWiSe.exe" (
    echo [OK] PROWiSe.exe found
    for %%i in (PROWiSe.exe) do echo     Size: %%~zi bytes, Date: %%~ti
) else (
    echo [ERROR] PROWiSe.exe not found in current directory
)

if exist "ModernMenu.dll" (
    echo [OK] ModernMenu.dll found
    for %%i in (ModernMenu.dll) do echo     Size: %%~zi bytes, Date: %%~ti
) else (
    echo [ERROR] ModernMenu.dll not found - REQUIRED
)

if exist "English.lng" (
    echo [OK] English.lng found
) else (
    echo [WARNING] English.lng not found
)

if exist "Russian.lng" (
    echo [OK] Russian.lng found
) else (
    echo [INFO] Russian.lng not found
)

echo.
echo ====================================
echo TROUBLESHOOTING RECOMMENDATIONS:
echo ====================================
echo.

echo If PROWiSe.exe fails with error 0xc0000142:
echo 1. Install missing Visual C++ Runtimes (see above)
echo 2. Try running as Administrator
echo 3. Try Windows XP compatibility mode
echo 4. Temporarily disable antivirus
echo 5. See INSTALLATION.md for detailed steps
echo.

echo If you get "Windows XP required" message:
echo - This is normal on newer Windows
echo - Click "Continue anyway" to proceed
echo.

echo SAVE THIS INFORMATION:
echo You can copy this window contents to send for support
echo (Right-click in window, select "Mark", select all text, press Enter)
echo.
pause

REM Try to create a log file
echo Writing system info to system_info.txt...
(
echo PROWiSe Manager System Information - %date% %time%
echo ====================================
echo.
ver
echo.
echo Architecture: %PROCESSOR_ARCHITECTURE%
if defined PROCESSOR_ARCHITEW6432 echo WOW64: %PROCESSOR_ARCHITEW6432%
echo User: %USERNAME%
echo SystemRoot: %SystemRoot%
echo.
if exist "%SystemRoot%\System32\msvcr80.dll" (echo VC2005: Found) else (echo VC2005: MISSING)
if exist "%SystemRoot%\System32\msvcr90.dll" (echo VC2008: Found) else (echo VC2008: MISSING)
if exist "PROWiSe.exe" (echo PROWiSe.exe: Found) else (echo PROWiSe.exe: NOT FOUND)
if exist "ModernMenu.dll" (echo ModernMenu.dll: Found) else (echo ModernMenu.dll: NOT FOUND)
) > system_info.txt

if exist system_info.txt (
    echo [OK] System information saved to system_info.txt
) else (
    echo [WARNING] Could not create system_info.txt
)

echo.
pause