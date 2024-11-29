@echo off

REM call Visual Studio Command Prompt
if exist "%PROGRAMFILES(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
	for /f "usebackq tokens=*" %%i in (`"%PROGRAMFILES(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
		set LatestVSInstallDir=%%i
	)
) else if exist "%PROGRAMFILES%\Microsoft Visual Studio\Installer\vswhere.exe" (
	for /f "usebackq tokens=*" %%i in (`"%PROGRAMFILES%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
		set LatestVSInstallDir=%%i
	)
)
if exist "%LatestVSInstallDir%\VC\Auxiliary\Build\vcvarsall.bat" (
	call "%LatestVSInstallDir%\VC\Auxiliary\Build\vcvarsall.bat" x86
) else if exist "%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat" (
	call "%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat" x86
) else if exist "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" (
	call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" x86
) else if exist "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" (
	call "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" x86
) else if exist "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" (
	call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" x86
) else (
	echo ERROR: Could not find environment variables for Visual Studio
	goto batchpause
)

REM clear nsis installation directory variable
set NSISDIR=
REM divide with _ of REG_SZ because the first column differs for every language, in Italian it is <Senza nome>, English: (Default), ...
for /F "tokens=1* delims=_" %%A in ('REG QUERY "HKLM\Software\NSIS" /ve 2^>NUL ^| FIND "REG_SZ"') do set NSISDIR=%%B
REM on 64 bit platforms try the 32 bit mode to read the registry
if "%NSISDIR%"=="" (
	for /F "tokens=1* delims=_" %%A in ('REG QUERY "HKLM\Software\NSIS" /ve /reg:32 2^>NUL ^| FIND "REG_SZ"') do set NSISDIR=%%B
)
REM now divide at space char
if NOT "%NSISDIR%"=="" (
	for /F "tokens=1*" %%A in ("%NSISDIR%") do set NSISDIR=%%B
)
REM check
if "%NSISDIR%"=="" (
	echo:
	echo ERROR: NSIS installation not found
	goto batchpause
)

REM get this script directory and change to it
set BATCHDIR=%~dp0
cd /D "%BATCHDIR%"

REM get current version 
for /F "tokens=3" %%V in (CurrentVersion.nsh) do set CURRENTVERSION=%%~V

REM delete possible old folders
echo:
echo **********************************************************************
echo ** Delete input and %CURRENTVERSION% folders (if existing)
echo **********************************************************************
rmdir /S /Q .\input >nul 2>&1
rmdir /S /Q .\%CURRENTVERSION% >nul 2>&1

REM 7-zip the source code
echo:
echo **********************************************************************
echo ** Compress source code
echo **********************************************************************
ping -n 5 127.0.0.1 >nul
cd ..
call 7Zip_Full.bat
cd "%BATCHDIR%"

REM create directories
echo:
echo **********************************************************************
echo ** Make input and %CURRENTVERSION% folders
echo **********************************************************************
mkdir .\input
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
mkdir .\%CURRENTVERSION%\ContaCam\japanese
mkdir .\%CURRENTVERSION%\FreeVimager
mkdir .\%CURRENTVERSION%\FreeVimager\english
mkdir .\%CURRENTVERSION%\FreeVimager\german
mkdir .\%CURRENTVERSION%\FreeVimager\italian
mkdir .\%CURRENTVERSION%\FreeVimager\russian
mkdir .\%CURRENTVERSION%\FreeVimager\spanish
mkdir .\%CURRENTVERSION%\FreeVimager\french
mkdir .\%CURRENTVERSION%\FreeVimager\portuguese
mkdir .\%CURRENTVERSION%\FreeVimager\chinese
mkdir .\%CURRENTVERSION%\FreeVimager\japanese

REM copy executables to input folder
echo:
echo **********************************************************************
echo ** Copy executables to input folder
echo **********************************************************************
copy ..\ContaCamService\Release\ContaCamService.exe .\input\
copy ..\Bin\ContaCam\ContaCam.exe .\input\
copy ..\Translation\ContaCamDEU.exe .\input\
copy ..\Translation\ContaCamITA.exe .\input\
copy ..\Translation\ContaCamRUS.exe .\input\
copy ..\Translation\ContaCamESN.exe .\input\
copy ..\Translation\ContaCamFRA.exe .\input\
copy ..\Translation\ContaCamPTB.exe .\input\
copy ..\Translation\ContaCamCHS.exe .\input\
copy ..\Translation\ContaCamJPN.exe .\input\
copy ..\Bin\FreeVimager\FreeVimager.exe .\input\
copy ..\Translation\FreeVimagerDEU.exe .\input\
copy ..\Translation\FreeVimagerITA.exe .\input\
copy ..\Translation\FreeVimagerRUS.exe .\input\
copy ..\Translation\FreeVimagerESN.exe .\input\
copy ..\Translation\FreeVimagerFRA.exe .\input\
copy ..\Translation\FreeVimagerPTB.exe .\input\
copy ..\Translation\FreeVimagerCHS.exe .\input\
copy ..\Translation\FreeVimagerJPN.exe .\input\

REM sign executables
echo:
echo **********************************************************************
echo ** Sign executables
echo **********************************************************************
signtool sign /n "Open Source Developer, Oliver Pfister" /t http://time.certum.pl/ /fd sha256 /v ^
.\input\ContaCamService.exe ^
.\input\ContaCam.exe ^
.\input\ContaCamDEU.exe ^
.\input\ContaCamITA.exe ^
.\input\ContaCamRUS.exe ^
.\input\ContaCamESN.exe ^
.\input\ContaCamFRA.exe ^
.\input\ContaCamPTB.exe ^
.\input\ContaCamCHS.exe ^
.\input\ContaCamJPN.exe ^
.\input\FreeVimager.exe ^
.\input\FreeVimagerDEU.exe ^
.\input\FreeVimagerITA.exe ^
.\input\FreeVimagerRUS.exe ^
.\input\FreeVimagerESN.exe ^
.\input\FreeVimagerFRA.exe ^
.\input\FreeVimagerPTB.exe ^
.\input\FreeVimagerCHS.exe ^
.\input\FreeVimagerJPN.exe

REM make installers
echo:
echo **********************************************************************
echo ** Make NSIS installers
echo **********************************************************************
echo ContaCam installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=English ContaCam.nsi
echo ContaCamDeu installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=German ContaCam.nsi
echo ContaCamIta installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Italian ContaCam.nsi
echo ContaCamRus installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Russian ContaCam.nsi
echo ContaCamEsn installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Spanish ContaCam.nsi
echo ContaCamFra installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=French ContaCam.nsi
echo ContaCamPtb installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Portuguese ContaCam.nsi
echo ContaCamChs installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=SimpChinese ContaCam.nsi
echo ContaCamJpn installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Japanese ContaCam.nsi
echo FreeVimager installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=English FreeVimager.nsi
echo FreeVimagerDeu installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=German FreeVimager.nsi
echo FreeVimagerIta installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Italian FreeVimager.nsi
echo FreeVimagerRus installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Russian FreeVimager.nsi
echo FreeVimagerEsn installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Spanish FreeVimager.nsi
echo FreeVimagerFra installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=French FreeVimager.nsi
echo FreeVimagerPtb installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Portuguese FreeVimager.nsi
echo FreeVimagerChs installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=SimpChinese FreeVimager.nsi
echo FreeVimagerJpn installer
"%NSISDIR%\makensis.exe" /V2 /DINSTALLER_LANGUAGE=Japanese FreeVimager.nsi

REM sign installers
echo:
echo **********************************************************************
echo ** Sign installers
echo **********************************************************************
signtool sign /n "Open Source Developer, Oliver Pfister" /t http://time.certum.pl/ /fd sha256 /v ^
.\ContaCam-%CURRENTVERSION%-Setup.exe ^
.\ContaCam-%CURRENTVERSION%-Setup-Deu.exe ^
.\ContaCam-%CURRENTVERSION%-Setup-Ita.exe ^
.\ContaCam-%CURRENTVERSION%-Setup-Rus.exe ^
.\ContaCam-%CURRENTVERSION%-Setup-Esn.exe ^
.\ContaCam-%CURRENTVERSION%-Setup-Fra.exe ^
.\ContaCam-%CURRENTVERSION%-Setup-Ptb.exe ^
.\ContaCam-%CURRENTVERSION%-Setup-Chs.exe ^
.\ContaCam-%CURRENTVERSION%-Setup-Jpn.exe ^
.\FreeVimager-%CURRENTVERSION%-Setup.exe ^
.\FreeVimager-%CURRENTVERSION%-Setup-Deu.exe ^
.\FreeVimager-%CURRENTVERSION%-Setup-Ita.exe ^
.\FreeVimager-%CURRENTVERSION%-Setup-Rus.exe ^
.\FreeVimager-%CURRENTVERSION%-Setup-Esn.exe ^
.\FreeVimager-%CURRENTVERSION%-Setup-Fra.exe ^
.\FreeVimager-%CURRENTVERSION%-Setup-Ptb.exe ^
.\FreeVimager-%CURRENTVERSION%-Setup-Chs.exe ^
.\FreeVimager-%CURRENTVERSION%-Setup-Jpn.exe

REM place all files into version folder
echo:
echo **********************************************************************
echo ** Place all files into %CURRENTVERSION%
echo **********************************************************************
copy ..\Bin\FreeVimager\FreeVimager.pdb .\%CURRENTVERSION%\source-code\FreeVimager.pdb
copy ..\Bin\ContaCam\ContaCam.pdb .\%CURRENTVERSION%\source-code\ContaCam.pdb
move ..\uimager_Full.7z .\%CURRENTVERSION%\source-code\contacam-freevimager-src-ver%CURRENTVERSION%.7z
move .\input\FreeVimager.exe .\%CURRENTVERSION%\FreeVimager\english\FreeVimager-%CURRENTVERSION%-Portable.exe
move .\input\FreeVimagerDEU.exe .\%CURRENTVERSION%\FreeVimager\german\FreeVimager-%CURRENTVERSION%-Portable-Deu.exe
move .\input\FreeVimagerITA.exe .\%CURRENTVERSION%\FreeVimager\italian\FreeVimager-%CURRENTVERSION%-Portable-Ita.exe
move .\input\FreeVimagerRUS.exe .\%CURRENTVERSION%\FreeVimager\russian\FreeVimager-%CURRENTVERSION%-Portable-Rus.exe
move .\input\FreeVimagerESN.exe .\%CURRENTVERSION%\FreeVimager\spanish\FreeVimager-%CURRENTVERSION%-Portable-Esn.exe
move .\input\FreeVimagerFRA.exe .\%CURRENTVERSION%\FreeVimager\french\FreeVimager-%CURRENTVERSION%-Portable-Fra.exe
move .\input\FreeVimagerPTB.exe .\%CURRENTVERSION%\FreeVimager\portuguese\FreeVimager-%CURRENTVERSION%-Portable-Ptb.exe
move .\input\FreeVimagerCHS.exe .\%CURRENTVERSION%\FreeVimager\chinese\FreeVimager-%CURRENTVERSION%-Portable-Chs.exe
move .\input\FreeVimagerJPN.exe .\%CURRENTVERSION%\FreeVimager\japanese\FreeVimager-%CURRENTVERSION%-Portable-Jpn.exe
move .\ContaCam-%CURRENTVERSION%-Setup.exe .\%CURRENTVERSION%\ContaCam\english\
move .\ContaCam-%CURRENTVERSION%-Setup-Deu.exe .\%CURRENTVERSION%\ContaCam\german\
move .\ContaCam-%CURRENTVERSION%-Setup-Ita.exe .\%CURRENTVERSION%\ContaCam\italian\
move .\ContaCam-%CURRENTVERSION%-Setup-Rus.exe .\%CURRENTVERSION%\ContaCam\russian\
move .\ContaCam-%CURRENTVERSION%-Setup-Esn.exe .\%CURRENTVERSION%\ContaCam\spanish\
move .\ContaCam-%CURRENTVERSION%-Setup-Fra.exe .\%CURRENTVERSION%\ContaCam\french\
move .\ContaCam-%CURRENTVERSION%-Setup-Ptb.exe .\%CURRENTVERSION%\ContaCam\portuguese\
move .\ContaCam-%CURRENTVERSION%-Setup-Chs.exe .\%CURRENTVERSION%\ContaCam\chinese\
move .\ContaCam-%CURRENTVERSION%-Setup-Jpn.exe .\%CURRENTVERSION%\ContaCam\japanese\
move .\FreeVimager-%CURRENTVERSION%-Setup.exe .\%CURRENTVERSION%\FreeVimager\english\
move .\FreeVimager-%CURRENTVERSION%-Setup-Deu.exe .\%CURRENTVERSION%\FreeVimager\german\
move .\FreeVimager-%CURRENTVERSION%-Setup-Ita.exe .\%CURRENTVERSION%\FreeVimager\italian\
move .\FreeVimager-%CURRENTVERSION%-Setup-Rus.exe .\%CURRENTVERSION%\FreeVimager\russian\
move .\FreeVimager-%CURRENTVERSION%-Setup-Esn.exe .\%CURRENTVERSION%\FreeVimager\spanish\
move .\FreeVimager-%CURRENTVERSION%-Setup-Fra.exe .\%CURRENTVERSION%\FreeVimager\french\
move .\FreeVimager-%CURRENTVERSION%-Setup-Ptb.exe .\%CURRENTVERSION%\FreeVimager\portuguese\
move .\FreeVimager-%CURRENTVERSION%-Setup-Chs.exe .\%CURRENTVERSION%\FreeVimager\chinese\
move .\FreeVimager-%CURRENTVERSION%-Setup-Jpn.exe .\%CURRENTVERSION%\FreeVimager\japanese\

REM delete input folder
echo:
echo **********************************************************************
echo ** Delete input folder
echo **********************************************************************
rmdir /S /Q .\input >nul 2>&1

REM exit
:batchpause
echo:
pause
