# PROWiSe Manager - Visual Studio 2019 Build Instructions

This document describes how to build PROWiSe Manager with Visual Studio 2019, having been ported from the original Borland C++ Builder 6 project.

## Prerequisites

- Visual Studio 2019 (Community, Professional, or Enterprise)
- Windows SDK 10.0 or later
- MASM (Microsoft Macro Assembler) - included with Visual Studio
- Git (for version control)

## Project Structure

The project has been converted from Borland C++ Builder (.bpr) to Visual Studio 2019 format:

- `PROWiSe.sln` - Visual Studio 2019 solution file
- `PROWiSe.vcxproj` - Main project file
- `PROWiSe.vcxproj.filters` - File organization filters for IDE

## Build Configurations

The project supports two build configurations:
- **Debug** - For development with debug symbols
- **Release** - Optimized build for distribution

Both configurations target Win32 (x86) architecture to maintain compatibility with the original application.

## Key Changes from Borland C++ Builder

### 1. Project Files
- Converted from `.bpr` (Borland Project) to `.vcxproj` (Visual Studio Project)
- Updated build settings to use MSVC compiler instead of Borland compiler
- Configured MASM for assembly file compilation

### 2. Library Dependencies
Original Borland libraries have been replaced with MSVC equivalents:
- `import32.lib` → `kernel32.lib`, `user32.lib`, etc.
- `cw32mt.lib` → Static runtime library (`/MT`)
- Added Windows-specific libraries: `comctl32.lib`, `netapi32.lib`

### 3. Assembly Files
- `AsmFuncs.asm` and `myFuncs.asm` configured for MASM
- Assembly functions properly declared with `extern "C"` linkage

### 4. Missing Dependencies
Created stub implementations for missing source files:
- `StringF.cpp` - String manipulation functions
- `HeadFuncs.cpp` - Core header functions and NT API wrappers
- `Run_Dlg.cpp` - Run dialog functionality
- `Processes_BtmPane.cpp` - Process bottom pane UI
- `SrvcGroups.cpp` - Service groups management
- `ServicesRemote.cpp` - Remote services functionality
- `WriteDataBin.cpp` - Binary data writing

### 5. Header Files
Created essential header files for missing dependencies:
- `crc32.h` - CRC32 checksum functions
- `TextStrings.h` - Text string definitions
- `HeaderStructs.h` - Structure definitions
- `FuncsDefines.h` - Function declarations
- `htmlhlp.h` - HTML Help API compatibility

## How to Build

### Using Visual Studio IDE
1. Open `PROWiSe.sln` in Visual Studio 2019
2. Select build configuration (Debug or Release)
3. Ensure platform is set to x86 (Win32)
4. Build → Build Solution (Ctrl+Shift+B)

### Using MSBuild (Command Line)
```cmd
# Debug build
MSBuild PROWiSe.sln /p:Configuration=Debug /p:Platform=Win32

# Release build  
MSBuild PROWiSe.sln /p:Configuration=Release /p:Platform=Win32
```

### Using Developer Command Prompt
```cmd
# Open Visual Studio 2019 Developer Command Prompt
cd /path/to/prowise-manager
devenv PROWiSe.sln /build "Release|Win32"
```

## Compiler Settings

### Preprocessor Definitions
- `_X86_` - Target x86 architecture
- `_WINDOWS` - Windows application
- `_NO_VCL` - No Visual Component Library (legacy from Borland)
- `_CRT_SECURE_NO_WARNINGS` - Disable CRT security warnings
- `WIN32` - Win32 API
- `_DEBUG` (Debug builds) / `NDEBUG` (Release builds)

### Runtime Library
- Debug: Multi-threaded Debug (`/MTd`)
- Release: Multi-threaded (`/MT`)

### C++ Language Standard
- **C++14** (`stdcpp14`) - Default standard for Visual Studio 2019
- **ConformanceMode enabled** - Ensures strict C++ standard compliance

### Warning Level
- Level 3 with SDL checks disabled for compatibility

## Assembly Integration

The project includes x86 assembly files that are compiled with MASM:
- `AsmFuncs.asm` - Process and system functions
- `myFuncs.asm` - Hash calculation and string functions

Assembly functions are properly declared in `FuncsDefines.h` with C linkage.

## Resource Files

The project includes Windows resource files:
- `PROWiSe_Rc.rc` - Main application resources
- `Job Dialog.rc` - Job dialog resources

## Recent Updates (C++ Standard Compliance)

The project has been updated to compile with Visual Studio 2019's default C++ standard:

1. **ConformanceMode enabled**: Project now uses strict C++ standard compliance
2. **C++14 Language Standard**: Explicitly set to match VS2019 default
3. **Variable Scoping Fixed**: Resolved C++ scoping issues in About.cpp

## Known Issues and Limitations

1. **Missing Implementations**: Many functions are currently stubs and need full implementation
2. **Assembly Compatibility**: Some assembly syntax may need adjustment for MASM
3. **String Functions**: Custom string functions need proper implementation
4. **NT API**: Some NT API calls may need updates for modern Windows

## Testing the Build

After successful compilation:
1. Ensure `PROWiSe.exe` is created in the output directory
2. Test basic application startup
3. Verify that resource files are properly embedded
4. Check that assembly functions link correctly

## Troubleshooting

### Common Build Errors

**MASM Errors**: Ensure MASM is properly installed with Visual Studio
```
Error: MASM.exe not found
Solution: Install "MSVC v142 - VS 2019 C++ x64/x86 build tools" workload
```

**Missing Header Files**: Ensure all custom headers are in the include path
```
Error: Cannot open include file 'FuncsDefines.h'
Solution: Verify file exists and include paths are correct
```

**Linker Errors**: Check that all object files and libraries are properly linked
```
Error: Unresolved external symbol
Solution: Verify function declarations match implementations
```

## Future Improvements

1. Complete implementation of stub functions
2. Update deprecated Windows API calls
3. Add proper error handling and logging
4. Modernize UI components for current Windows versions
5. Add unit tests for core functionality

## Support

For build issues specific to the Visual Studio 2019 conversion, check:
1. Project configuration settings match this document
2. All required files are present
3. Visual Studio 2019 is properly installed with C++ workload
4. Windows SDK is available

Original application documentation may still apply for functionality questions.