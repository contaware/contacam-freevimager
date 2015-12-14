@echo off
SET batchdir=%~dp0
SET dumpdir=%APPDATA%\Contaware\ContaCam

:procdump_download
if exist "%batchdir%procdump.exe" goto vmmap_download
echo procdump.exe from Sysinternals is necessary, please save/move it to:
echo "%batchdir%"
pause
start "" "http://live.sysinternals.com/procdump.exe"
:wait_procdump_download
ping -n 2 127.0.0.1 >nul
if not exist "%batchdir%procdump.exe" goto wait_procdump_download
ping -n 2 127.0.0.1 >nul

:vmmap_download
if exist "%batchdir%vmmap.exe" goto procdump_exec
echo vmmap.exe from Sysinternals is necessary, please save/move it to:
echo "%batchdir%"
pause
start "" "http://live.sysinternals.com/vmmap.exe"
:wait_vmmap_download
ping -n 2 127.0.0.1 >nul
if not exist "%batchdir%vmmap.exe" goto wait_vmmap_download
ping -n 2 127.0.0.1 >nul

:procdump_exec
mkdir "%dumpdir%" >nul 2>&1
"%batchdir%procdump.exe" -accepteula -e -mp -x "%dumpdir%" "%batchdir%ContaCam.exe"
start "" "%dumpdir%"
pause