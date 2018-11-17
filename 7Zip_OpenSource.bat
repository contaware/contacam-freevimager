@echo off

REM Configure 7-Zip installation directory
set exedir=C:\Program Files\7-Zip

REM Delete old version if existing
del .\uimager_opensource.7z >nul 2>&1

REM 7-Zip it
"%exedir%\7z.exe" a -r -mx9 -t7z uimager_opensource.7z * -xr-!?svn\ -xr-!.vs\ -xr-!bin\ -xr-!ipch\ -xr-!obj\ -xr-!ContaCamService\ -xr-!doc\ -xr-!Setup\ -xr-!Translation\ -xr-!7Zip_Full.bat -xr-!7Zip_OpenSource.bat
