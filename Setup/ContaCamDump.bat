@echo off

REM If ContaCam crashes then run it through this .bat file,
REM when it stops working send us the generated .dmp file

set batchdir=%~dp0
set dumpdir=%APPDATA%\Contaware\ContaCam

:procdump_download
if exist "%batchdir%procdump.exe" goto procdump_exec
echo procdump.exe from Sysinternals is necessary, please save/move it to:
echo "%batchdir%"
pause
start "" "https://live.sysinternals.com/procdump.exe"
:wait_procdump_download
ping -n 2 127.0.0.1 >nul
if not exist "%batchdir%procdump.exe" goto wait_procdump_download
ping -n 2 127.0.0.1 >nul

:procdump_exec
mkdir "%dumpdir%" >nul 2>&1
"%batchdir%procdump.exe" -accepteula -e -mp -x "%dumpdir%" "%batchdir%ContaCam.exe"
start "" "%dumpdir%"
pause