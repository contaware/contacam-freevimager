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
echo Deleting %CURRENTVERSION% output folder if existing
rmdir /S /Q .\%CURRENTVERSION% >nul 2>&1

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
mkdir .\%CURRENTVERSION%\ContaCam
mkdir .\%CURRENTVERSION%\ContaCam\english
mkdir .\%CURRENTVERSION%\ContaCam\german
mkdir .\%CURRENTVERSION%\ContaCam\italian
mkdir .\%CURRENTVERSION%\ContaCam\russian
mkdir .\%CURRENTVERSION%\ContaCam\spanish
mkdir .\%CURRENTVERSION%\ContaCam\french
mkdir .\%CURRENTVERSION%\ContaCam\portuguese
mkdir .\%CURRENTVERSION%\ContaCam\chinese
mkdir .\%CURRENTVERSION%\FreeVimager
mkdir .\%CURRENTVERSION%\FreeVimager\english
mkdir .\%CURRENTVERSION%\FreeVimager\german
mkdir .\%CURRENTVERSION%\FreeVimager\italian
mkdir .\%CURRENTVERSION%\FreeVimager\russian
mkdir .\%CURRENTVERSION%\FreeVimager\spanish
mkdir .\%CURRENTVERSION%\FreeVimager\french
mkdir .\%CURRENTVERSION%\FreeVimager\portuguese
mkdir .\%CURRENTVERSION%\FreeVimager\chinese

REM Move the 7-zipped source code
echo Move source code
move ..\uimager_Full.7z .\%CURRENTVERSION%\source-code\contacam-freevimager-src-ver%CURRENTVERSION%.7z

REM Copy the .pdb files to the source-code folder
echo Copy FreeVimager.pdb
copy ..\bin\FreeVimager\FreeVimager.pdb .\%CURRENTVERSION%\source-code\FreeVimager.pdb
echo Copy ContaCam.pdb
copy ..\bin\ContaCam\ContaCam.pdb .\%CURRENTVERSION%\source-code\ContaCam.pdb

REM Copy FreeVimager portable versions
echo Copy FreeVimager.exe
copy ..\bin\FreeVimager\FreeVimager.exe .\%CURRENTVERSION%\FreeVimager\english\FreeVimager-%CURRENTVERSION%-Portable.exe
echo Copy FreeVimagerDeu.exe
copy ..\Translation\FreeVimagerDEU.exe .\%CURRENTVERSION%\FreeVimager\german\FreeVimager-%CURRENTVERSION%-Portable-Deu.exe
echo Copy FreeVimagerIta.exe
copy ..\Translation\FreeVimagerITA.exe .\%CURRENTVERSION%\FreeVimager\italian\FreeVimager-%CURRENTVERSION%-Portable-Ita.exe
echo Copy FreeVimagerRus.exe
copy ..\Translation\FreeVimagerRUS.exe .\%CURRENTVERSION%\FreeVimager\russian\FreeVimager-%CURRENTVERSION%-Portable-Rus.exe
echo Copy FreeVimagerEsn.exe
copy ..\Translation\FreeVimagerESN.exe .\%CURRENTVERSION%\FreeVimager\spanish\FreeVimager-%CURRENTVERSION%-Portable-Esn.exe
echo Copy FreeVimagerFra.exe
copy ..\Translation\FreeVimagerFRA.exe .\%CURRENTVERSION%\FreeVimager\french\FreeVimager-%CURRENTVERSION%-Portable-Fra.exe
echo Copy FreeVimagerPtb.exe
copy ..\Translation\FreeVimagerPTB.exe .\%CURRENTVERSION%\FreeVimager\portuguese\FreeVimager-%CURRENTVERSION%-Portable-Ptb.exe
echo Copy FreeVimagerChs.exe
copy ..\Translation\FreeVimagerCHS.exe .\%CURRENTVERSION%\FreeVimager\chinese\FreeVimager-%CURRENTVERSION%-Portable-Chs.exe

REM Make all the different ContaCam installers
echo NSIS make ContaCam installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=English ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup.exe .\%CURRENTVERSION%\ContaCam\english\
echo NSIS make ContaCamDeu installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=German ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Deu.exe .\%CURRENTVERSION%\ContaCam\german\
echo NSIS make ContaCamIta installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Italian ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Ita.exe .\%CURRENTVERSION%\ContaCam\italian\
echo NSIS make ContaCamRus installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Russian ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Rus.exe .\%CURRENTVERSION%\ContaCam\russian\
echo NSIS make ContaCamEsn installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Spanish ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Esn.exe .\%CURRENTVERSION%\ContaCam\spanish\
echo NSIS make ContaCamFra installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=French ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Fra.exe .\%CURRENTVERSION%\ContaCam\french\
echo NSIS make ContaCamPtb installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Portuguese ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Ptb.exe .\%CURRENTVERSION%\ContaCam\portuguese\
echo NSIS make ContaCamChs installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=SimpChinese ContaCam.nsi
move .\ContaCam-%CURRENTVERSION%-Setup-Chs.exe .\%CURRENTVERSION%\ContaCam\chinese\

REM Make all the different FreeVimager installers
echo NSIS make FreeVimager installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=English FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup.exe .\%CURRENTVERSION%\FreeVimager\english\
echo NSIS make FreeVimagerDeu installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=German FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Deu.exe .\%CURRENTVERSION%\FreeVimager\german\
echo NSIS make FreeVimagerIta installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Italian FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Ita.exe .\%CURRENTVERSION%\FreeVimager\italian\
echo NSIS make FreeVimagerRus installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Russian FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Rus.exe .\%CURRENTVERSION%\FreeVimager\russian\
echo NSIS make FreeVimagerEsn installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Spanish FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Esn.exe .\%CURRENTVERSION%\FreeVimager\spanish\
echo NSIS make FreeVimagerFra installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=French FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Fra.exe .\%CURRENTVERSION%\FreeVimager\french\
echo NSIS make FreeVimagerPtb installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Portuguese FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Ptb.exe .\%CURRENTVERSION%\FreeVimager\portuguese\
echo NSIS make FreeVimagerChs installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=SimpChinese FreeVimager.nsi
move .\FreeVimager-%CURRENTVERSION%-Setup-Chs.exe .\%CURRENTVERSION%\FreeVimager\chinese\

REM exit
:batchpause
pause