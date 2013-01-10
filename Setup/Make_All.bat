@echo off

REM Configure NSIS installation directory
set nsisdir=C:\Program Files\NSIS

REM Create output directories
for /F "tokens=3" %%V in (CurrentVersion.nsh) do set CURRENTVERSION=%%~V
rmdir /S /Q .\%CURRENTVERSION%
mkdir .\%CURRENTVERSION%
mkdir .\%CURRENTVERSION%\german
mkdir .\%CURRENTVERSION%\italian
mkdir .\%CURRENTVERSION%\russian

REM Copy FreeVimager standalone versions
copy ..\bin\FreeVimager\FreeVimager.exe .\%CURRENTVERSION%\FreeVimager-%CURRENTVERSION%.exe
copy ..\bin\FreeVimagerw\FreeVimagerw.exe .\%CURRENTVERSION%\FreeVimagerUnicode-%CURRENTVERSION%.exe
copy ..\Translation\FreeVimagerwDEU.exe .\%CURRENTVERSION%\german\FreeVimagerDeu-%CURRENTVERSION%.exe
copy ..\Translation\FreeVimagerwITA.exe .\%CURRENTVERSION%\italian\FreeVimagerIta-%CURRENTVERSION%.exe
copy ..\Translation\FreeVimagerwRUS.exe .\%CURRENTVERSION%\russian\FreeVimagerRus-%CURRENTVERSION%.exe

REM Make all the different ContaCam installers
echo NSIS make ContaCam installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_WIN9X /DINSTALLER_NT /DWITH_TUTORIALS ContaCam.nsi
echo NSIS make ContaCam no tutorials installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_WIN9X /DINSTALLER_NT ContaCam.nsi
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
