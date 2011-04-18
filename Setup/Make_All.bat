@echo off

REM Configure NSIS installation directory
set nsisdir=C:\Program Files\NSIS

REM Start making FreeVimager UPXs
start "UPX FreeVimager" /MIN cmd.exe /C MakeUPX_FreeVimager.bat
start "UPX FreeVimagerw" /MIN cmd.exe /C MakeUPX_FreeVimagerw.bat
start "UPX FreeVimagerDeu" /MIN cmd.exe /C MakeUPX_FreeVimagerDeu.bat
start "UPX FreeVimagerIta" /MIN cmd.exe /C MakeUPX_FreeVimagerIta.bat
start "UPX FreeVimagerRus" /MIN cmd.exe /C MakeUPX_FreeVimagerRus.bat

REM Update ActiveX directory (this must be done before starting with all the ContaCam nsis)
call Make_RemoteCam.bat

REM Make the ffmpeg source code
echo NSIS make ffmpeg source installer
"%nsisdir%\makensis.exe" /V2 FfmpegSource.nsi

REM Make all the different ContaCam installers
echo NSIS make ContaCam installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_NT /DWITH_TUTORIALS ContaCam.nsi
echo NSIS make ContaCam no tutorials installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_NT ContaCam.nsi
echo NSIS make ContaCam win all installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_WIN9X /DINSTALLER_NT /DWITH_TUTORIALS ContaCam.nsi
echo NSIS make ContaCamDeu installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_NT ContaCamDeu.nsi
echo NSIS make ContaCamIta installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_NT ContaCamIta.nsi
echo NSIS make ContaCamRus installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_NT ContaCamRus.nsi

REM Make all the different FreeVimager installers
echo NSIS make FreeVimager installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_WIN9X /DINSTALLER_NT /DWITH_TUTORIALS FreeVimager.nsi
echo NSIS make FreeVimager no tutorials installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_WIN9X /DINSTALLER_NT FreeVimager.nsi
echo NSIS make FreeVimagerDeu installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_NT FreeVimagerDeu.nsi
echo NSIS make FreeVimagerIta installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_NT FreeVimagerIta.nsi
echo NSIS make FreeVimagerRus installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_NT FreeVimagerRus.nsi

pause
