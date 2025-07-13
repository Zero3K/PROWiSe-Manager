@echo off
REM validate_vs2019_project.bat
REM Windows batch script to validate the Visual Studio 2019 project conversion

echo === PROWiSe Manager - Visual Studio 2019 Project Validation ===
echo.

echo Checking project files...
if exist "PROWiSe.sln" (echo ✓ PROWiSe.sln - Found) else (echo ✗ PROWiSe.sln - Missing)
if exist "PROWiSe.vcxproj" (echo ✓ PROWiSe.vcxproj - Found) else (echo ✗ PROWiSe.vcxproj - Missing)
if exist "PROWiSe.vcxproj.filters" (echo ✓ PROWiSe.vcxproj.filters - Found) else (echo ✗ PROWiSe.vcxproj.filters - Missing)
echo.

echo Checking key source files...
if exist "PROWiSe.cpp" (echo ✓ PROWiSe.cpp - Found) else (echo ✗ PROWiSe.cpp - Missing)
if exist "AsmFuncs.asm" (echo ✓ AsmFuncs.asm - Found) else (echo ✗ AsmFuncs.asm - Missing)
if exist "myFuncs.asm" (echo ✓ myFuncs.asm - Found) else (echo ✗ myFuncs.asm - Missing)
if exist "PROWiSe_Rc.rc" (echo ✓ PROWiSe_Rc.rc - Found) else (echo ✗ PROWiSe_Rc.rc - Missing)
echo.

echo Checking created header files...
if exist "FuncsDefines.h" (echo ✓ FuncsDefines.h - Created) else (echo ✗ FuncsDefines.h - Missing)
if exist "HeaderStructs.h" (echo ✓ HeaderStructs.h - Created) else (echo ✗ HeaderStructs.h - Missing)
if exist "crc32.h" (echo ✓ crc32.h - Created) else (echo ✗ crc32.h - Missing)
if exist "TextStrings.h" (echo ✓ TextStrings.h - Created) else (echo ✗ TextStrings.h - Missing)
echo.

echo Checking for Visual Studio 2019...
where devenv >nul 2>&1
if %errorlevel% == 0 (
    echo ✓ Visual Studio found in PATH
) else (
    echo ⚠ Visual Studio not found in PATH - may need to run from Developer Command Prompt
)

where MSBuild >nul 2>&1
if %errorlevel% == 0 (
    echo ✓ MSBuild found in PATH
) else (
    echo ⚠ MSBuild not found in PATH - may need to run from Developer Command Prompt
)
echo.

echo === Ready to Build ===
echo To build the project:
echo 1. Open PROWiSe.sln in Visual Studio 2019
echo 2. Select Win32 platform and Debug or Release configuration
echo 3. Press Ctrl+Shift+B to build
echo.
echo Alternative command line build:
echo   MSBuild PROWiSe.sln /p:Configuration=Release /p:Platform=Win32
echo.
echo See BUILD_VS2019.md for detailed instructions.
echo.
pause