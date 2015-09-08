@echo off

REM clear nsis installation directory variable
set NSISDIR=
REM divide with _ of REG_SZ because the first column differs for every language, in Italian it is <Senza nome>, English: (Default), ...
for /F "tokens=1* delims=_" %%A in ('REG QUERY "HKLM\Software\NSIS\Unicode" /ve 2^>NUL ^| FIND "REG_SZ"') do set NSISDIR=%%B
REM on 64 bit platforms try the 32 bit mode to read the registry
if "%NSISDIR%"=="" (
	for /F "tokens=1* delims=_" %%A in ('REG QUERY "HKLM\Software\NSIS\Unicode" /ve /reg:32 2^>NUL ^| FIND "REG_SZ"') do set NSISDIR=%%B
)
REM now divide at space char
if NOT "%NSISDIR%"=="" (
	for /F "tokens=1*" %%A in ("%NSISDIR%") do set NSISDIR=%%B
)
REM check
if "%NSISDIR%"=="" (
	echo ERROR: Unicode NSIS installation not found!
	goto batchpause    
)

REM get this script directory
set BATCHDIR=%~dp0

REM get current version 
for /F "tokens=3" %%V in (CurrentVersion.nsh) do set CURRENTVERSION=%%~V

REM Delete possible old output folder
echo Deleting %CURRENTVERSION% output folder
rmdir /S /Q .\%CURRENTVERSION%

REM 7-zip the source code
echo Starting to compress source code
ping -n 5 127.0.0.1 >nul
cd ..
call 7Zip_Full.bat
cd "%BATCHDIR%"

REM Create output directories
echo Making %CURRENTVERSION% output folder
mkdir .\%CURRENTVERSION%
mkdir .\%CURRENTVERSION%\source-code
mkdir .\%CURRENTVERSION%\english
mkdir .\%CURRENTVERSION%\german
mkdir .\%CURRENTVERSION%\italian
mkdir .\%CURRENTVERSION%\russian
mkdir .\%CURRENTVERSION%\bulgarian
mkdir .\%CURRENTVERSION%\vietnamese

REM Move the 7-zipped source code
echo Move source code
move ..\uimager_Full.7z .\%CURRENTVERSION%\source-code\contacam-freevimager-src-ver%CURRENTVERSION%.7z

REM Copy FreeVimager portable versions
echo Copy FreeVimager.exe
copy ..\bin\FreeVimager\FreeVimager.exe .\%CURRENTVERSION%\english\FreeVimager-%CURRENTVERSION%-Portable.exe
echo Copy FreeVimagerDeu.exe
copy ..\Translation\FreeVimagerDEU.exe .\%CURRENTVERSION%\german\FreeVimager-%CURRENTVERSION%-Portable-Deu.exe
echo Copy FreeVimagerIta.exe
copy ..\Translation\FreeVimagerITA.exe .\%CURRENTVERSION%\italian\FreeVimager-%CURRENTVERSION%-Portable-Ita.exe
echo Copy FreeVimagerRus.exe
copy ..\Translation\FreeVimagerRUS.exe .\%CURRENTVERSION%\russian\FreeVimager-%CURRENTVERSION%-Portable-Rus.exe
echo Copy FreeVimagerBgr.exe
copy ..\Translation\FreeVimagerBGR.exe .\%CURRENTVERSION%\bulgarian\FreeVimager-%CURRENTVERSION%-Portable-Bgr.exe
echo Copy FreeVimagerVit.exe
copy ..\Translation\FreeVimagerVIT.exe .\%CURRENTVERSION%\vietnamese\FreeVimager-%CURRENTVERSION%-Portable-Vit.exe

REM Make all the different ContaCam installers
echo NSIS make ContaCam installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=English ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup.exe .\%CURRENTVERSION%\english\
echo NSIS make ContaCamDeu installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=German ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Deu.exe .\%CURRENTVERSION%\german\
echo NSIS make ContaCamIta installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Italian ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Ita.exe .\%CURRENTVERSION%\italian\
echo NSIS make ContaCamRus installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Russian ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Rus.exe .\%CURRENTVERSION%\russian\
echo NSIS make ContaCamBgr installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Bulgarian ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Bgr.exe .\%CURRENTVERSION%\bulgarian\
echo NSIS make ContaCamVit installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Vietnamese ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Vit.exe .\%CURRENTVERSION%\vietnamese\

REM Make all the different FreeVimager installers
echo NSIS make FreeVimager installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=English FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup.exe .\%CURRENTVERSION%\english\
echo NSIS make FreeVimagerDeu installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=German FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Deu.exe .\%CURRENTVERSION%\german\
echo NSIS make FreeVimagerIta installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Italian FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Ita.exe .\%CURRENTVERSION%\italian\
echo NSIS make FreeVimagerRus installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Russian FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Rus.exe .\%CURRENTVERSION%\russian\
echo NSIS make FreeVimagerBgr installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Bulgarian FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Bgr.exe .\%CURRENTVERSION%\bulgarian\
echo NSIS make FreeVimagerVit installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Vietnamese FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Vit.exe .\%CURRENTVERSION%\vietnamese\

REM exit
:batchpause
pause