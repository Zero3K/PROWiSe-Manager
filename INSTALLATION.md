# PROWiSe Manager - Installation and Troubleshooting Guide

## System Requirements
- Windows XP Service Pack 3 or later
- Windows Vista/7/8/8.1/10/11 (32-bit or 64-bit)
- Visual C++ Runtime Libraries (see below)

## Installation

### Quick Start
1. Download all files to a folder
2. Run `check_requirements.bat` to verify your system
3. Install any missing runtime libraries
4. Run `PROWiSe.exe`

### If You Get Error 0xc0000142 (Application failed to start correctly)

This error typically means missing Visual C++ runtime libraries. Follow these steps:

#### Step 1: Install Required Runtimes
Download and install the Microsoft Visual C++ Redistributables:

- **Visual C++ 2005 SP1 Redistributable (x86)**
  - Download: https://www.microsoft.com/en-us/download/details.aspx?id=26347
  - Required for applications built with older development tools

- **Visual C++ 2008 SP1 Redistributable (x86)**  
  - Download: https://www.microsoft.com/en-us/download/details.aspx?id=29
  - Additional runtime support

#### Step 2: Try Compatibility Mode (Windows 7/8/10/11)
If the error persists after installing runtimes:

1. Right-click on `PROWiSe.exe`
2. Select **Properties**
3. Click the **Compatibility** tab
4. Check "**Run this program in compatibility mode for:**"
5. Select "**Windows XP (Service Pack 3)**"
6. Check "**Run this program as an administrator**"
7. Click **OK**

#### Step 3: Alternative Solutions
- Try running as Administrator (right-click → "Run as administrator")
- Disable antivirus temporarily during first run
- Ensure Windows is fully updated

## Troubleshooting Other Issues

### Application Starts But Shows "Windows XP Required" Message
This is normal on newer Windows versions. Click "Continue anyway" to proceed.

### Missing DLL Errors
- Ensure `ModernMenu.dll` is in the same folder as `PROWiSe.exe`
- Install the Visual C++ runtimes listed above

### Performance Issues
- The application was designed for older systems
- Performance may vary on modern hardware
- Try running in compatibility mode

## Files Included
- `PROWiSe.exe` - Main application
- `ModernMenu.dll` - UI library (keep in same folder)
- `Russian.lng` - Russian language file
- `Russian.chm` - Russian help file  
- `English.lng` - English language file (if available)
- `check_requirements.bat` - System checker utility

## Command Line Options
- `/drkey` - Delete registry key
- `/hide` - Start hidden
- `/nosv` - Don't save settings
- `/lng=Russian` - Use Russian.lng language file
- `/runas` - Show logon dialog
- `/runasadmin` - Run as administrator
- `/logon` - Enable logon options

## Support
This is a legacy application (last updated 2006) provided as-is. The troubleshooting steps above resolve most compatibility issues with modern Windows versions.