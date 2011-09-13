@echo off

REM Configure 7-Zip installation directory
set exedir=C:\Program Files\7-Zip

REM Delete old version
del .\uimager_OnlyTutorials.7z

REM 7-Zip it
"%exedir%\7z.exe" a -r -mx9 -t7z uimager_OnlyTutorials.7z Tutorials\* -xr!?svn\ -x!uimager_Full.7z -x!uimager_NoTutorials.7z

pause
