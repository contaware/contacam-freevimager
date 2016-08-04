@echo off

REM Configure 7-Zip installation directory
set exedir=C:\Program Files\7-Zip

REM Delete old version if existing
del .\uimager_Full.7z >nul 2>&1

REM 7-Zip it
"%exedir%\7z.exe" a -r -mx9 -t7z uimager_Full.7z * -xr-!uImagervs2010.sdf -xr-!uImagervs2015.VC.db -xr-!?svn\ -xr-!bin\ -xr-!ipch\ -xr-!obj\
