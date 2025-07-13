#!/bin/bash
# validate_vs2019_project.sh
# Script to validate the Visual Studio 2019 project conversion

echo "=== PROWiSe Manager - Visual Studio 2019 Project Validation ==="
echo

# Check for required project files
echo "Checking project files..."
required_files=(
    "PROWiSe.sln"
    "PROWiSe.vcxproj"
    "PROWiSe.vcxproj.filters"
)

for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file - Found"
    else
        echo "✗ $file - Missing"
    fi
done
echo

# Check for source files referenced in project
echo "Checking source files..."
source_files=(
    "PROWiSe.cpp"
    "AsmFuncs.asm"
    "myFuncs.asm"
    "PROWiSe_Rc.rc"
    "StringF.cpp"
    "HeadFuncs.cpp"
    "FuncsDefines.h"
    "HeaderStructs.h"
)

missing_files=0
for file in "${source_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file - Found"
    else
        echo "✗ $file - Missing"
        ((missing_files++))
    fi
done
echo

# Check for created stub files
echo "Checking created stub files..."
stub_files=(
    "Run_Dlg.cpp"
    "Processes_BtmPane.cpp"
    "SrvcGroups.cpp"
    "ServicesRemote.cpp"
    "WriteDataBin.cpp"
    "crc32.h"
    "TextStrings.h"
)

for file in "${stub_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file - Created"
    else
        echo "✗ $file - Missing"
        ((missing_files++))
    fi
done
echo

# Validate project file structure
echo "Validating project file content..."

if grep -q "PlatformToolset.*v142" PROWiSe.vcxproj; then
    echo "✓ Visual Studio 2019 toolset (v142) configured"
else
    echo "✗ Visual Studio 2019 toolset not found"
fi

if grep -q "MASM.*Include" PROWiSe.vcxproj; then
    echo "✓ MASM build rules included"
else
    echo "✗ MASM build rules not configured"
fi

if grep -q "CharacterSet.*MultiByte" PROWiSe.vcxproj; then
    echo "✓ Character set configured for compatibility"
else
    echo "✗ Character set not properly configured"
fi

if grep -q "_X86_.*_WINDOWS.*_NO_VCL" PROWiSe.vcxproj; then
    echo "✓ Preprocessor definitions match original project"
else
    echo "✗ Preprocessor definitions missing or incorrect"
fi

echo

# Check assembly files for MASM compatibility
echo "Checking assembly file compatibility..."

if grep -q "\.586p" AsmFuncs.asm && grep -q "model flat" AsmFuncs.asm; then
    echo "✓ Assembly files use MASM-compatible syntax"
else
    echo "✗ Assembly files may need syntax updates for MASM"
fi

echo

# Summary
echo "=== Validation Summary ==="
if [ $missing_files -eq 0 ]; then
    echo "✓ All required files are present"
    echo "✓ Project appears ready for Visual Studio 2019 compilation"
    echo
    echo "Next steps:"
    echo "1. Open PROWiSe.sln in Visual Studio 2019"
    echo "2. Select Win32 platform and desired configuration (Debug/Release)"
    echo "3. Build the solution (Ctrl+Shift+B)"
    echo "4. Address any compilation errors that arise"
    echo "5. Implement missing functionality in stub files as needed"
else
    echo "✗ $missing_files file(s) missing - please check project setup"
fi

echo
echo "For detailed build instructions, see BUILD_VS2019.md"