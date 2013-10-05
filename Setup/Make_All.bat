@echo off

REM clear variable
set nsisdir=
REM divide with _ of REG_SZ because the first column differs for every language, in Italian it is <Senza nome>, English: (Default), ...
for /F "tokens=1* delims=_" %%A in ('REG QUERY "HKLM\Software\NSIS\Unicode" /ve 2^>NUL ^| FIND "REG_SZ"') do set nsisdir=%%B
REM on 64 bit platforms try the 32 bit mode to read the registry
if "%nsisdir%"=="" (
	for /F "tokens=1* delims=_" %%A in ('REG QUERY "HKLM\Software\NSIS\Unicode" /ve /reg:32 2^>NUL ^| FIND "REG_SZ"') do set nsisdir=%%B
)
REM now divide at space char
if NOT "%nsisdir%"=="" (
	for /F "tokens=1*" %%A in ("%nsisdir%") do set nsisdir=%%B
)
REM check
if "%nsisdir%"=="" (
	echo ERROR: Unicode NSIS installation not found!
	goto batchpause    
)


REM Create output directories
for /F "tokens=3" %%V in (CurrentVersion.nsh) do set CURRENTVERSION=%%~V
echo Deleting %CURRENTVERSION% output folder
rmdir /S /Q .\%CURRENTVERSION%
echo Making %CURRENTVERSION% output folder
mkdir .\%CURRENTVERSION%
mkdir .\%CURRENTVERSION%\german
mkdir .\%CURRENTVERSION%\italian
mkdir .\%CURRENTVERSION%\russian


REM Copy FreeVimager standalone versions
echo Copy FreeVimager.exe
copy ..\bin\FreeVimager\FreeVimager.exe .\%CURRENTVERSION%\FreeVimager-%CURRENTVERSION%.exe
echo Copy FreeVimagerDeu.exe
copy ..\Translation\FreeVimagerDEU.exe .\%CURRENTVERSION%\german\FreeVimagerDeu-%CURRENTVERSION%.exe
echo Copy FreeVimagerIta.exe
copy ..\Translation\FreeVimagerITA.exe .\%CURRENTVERSION%\italian\FreeVimagerIta-%CURRENTVERSION%.exe
echo Copy FreeVimagerRus.exe
copy ..\Translation\FreeVimagerRUS.exe .\%CURRENTVERSION%\russian\FreeVimagerRus-%CURRENTVERSION%.exe


REM Make all the different ContaCam installers
echo NSIS make ContaCam installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=English ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup.exe .\%CURRENTVERSION%\
echo NSIS make ContaCamDeu installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=German ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Deu.exe .\%CURRENTVERSION%\german\
echo NSIS make ContaCamIta installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Italian ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Ita.exe .\%CURRENTVERSION%\italian\
echo NSIS make ContaCamRus installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Russian ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Rus.exe .\%CURRENTVERSION%\russian\


REM Make all the different FreeVimager installers
echo NSIS make FreeVimager installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=English FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup.exe .\%CURRENTVERSION%\
echo NSIS make FreeVimagerDeu installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=German FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Deu.exe .\%CURRENTVERSION%\german\
echo NSIS make FreeVimagerIta installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Italian FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Ita.exe .\%CURRENTVERSION%\italian\
echo NSIS make FreeVimagerRus installer
"%nsisdir%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Russian FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Rus.exe .\%CURRENTVERSION%\russian\


REM exit
:batchpause
pause