
if exist MenuLeftBorderDraw.obj del MenuLeftBorderDraw.obj

C:\Assemblers\MASM\masm32\bin\ml /c MenuLeftBorderDraw.asm

REM C:\Assemblers\coff2omf.exe MenuLeftBorderDraw.obj

copy /y MenuLeftBorderDraw.obj ..\

pause