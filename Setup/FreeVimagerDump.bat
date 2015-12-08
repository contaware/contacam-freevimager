@echo off
SET batchdir=%~dp0
SET dumpdir=%APPDATA%\Contaware\FreeVimager
if exist "%batchdir%procdump.exe" (
	mkdir "%dumpdir%"
	"%batchdir%procdump.exe" -accepteula -e -mp -x "%dumpdir%" "%batchdir%FreeVimager.exe"
	start "" "%dumpdir%"
	pause
) else (
	echo procdump.exe from Sysinternals is necessary, please save/move it to:
	echo "%batchdir%"
	pause
	start "" "http://live.sysinternals.com/procdump.exe"
)