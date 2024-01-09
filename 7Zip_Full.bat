@echo off

REM Configure 7-Zip installation directory
set exedir=C:\Program Files\7-Zip

REM Delete old version if existing
del .\uimager_Full.7z >nul 2>&1

REM 7-Zip it
"%exedir%\7z.exe" a -mx=9 uimager_Full.7z * -xr-!.svn\ -xr-!.vs\ -xr-!bin\ -xr-!obj\
