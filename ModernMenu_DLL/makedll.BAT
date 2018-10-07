@echo off


C:\Assemblers\MASM\MASM32\BIN\Rc.exe /v rsrc.rc
C:\Assemblers\MASM\MASM32\BIN\Cvtres.exe /machine:ix86 rsrc.res

if exist ModernMenu.obj del ModernMenu.obj
if exist ModernMenu.dll del ModernMenu.dll

C:\Assemblers\MASM\masm32\bin\ml /c /coff ModernMenu.asm

C:\Assemblers\MASM\masm32\bin\Link /SUBSYSTEM:WINDOWS /DLL /DEF:ModernMenu.def ModernMenu.obj rsrc.obj

copy /y ModernMenu.dll ..\

pause