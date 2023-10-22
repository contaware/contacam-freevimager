@echo off

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

REM create directories
echo:
echo **********************************************************************
echo ** Make input and %CURRENTVERSION% folders
echo **********************************************************************
mkdir .\input
mkdir .\%CURRENTVERSION%

REM copy executables to input folder
echo:
echo **********************************************************************
echo ** Copy executables to input folder
echo **********************************************************************
copy ..\ContaCamService\Release\ContaCamService.exe .\input\
copy ..\Bin\ContaCam\ContaCam.exe .\input\
copy ..\Bin\FreeVimager\FreeVimager.exe .\input\

REM make ContaCam installer
echo:
echo **********************************************************************
echo ** Make NSIS ContaCam installer
echo **********************************************************************
echo Please verify messages in opened window and then close it to continue.
"%NSISDIR%\makensisw.exe" ContaCam.nsi

REM make FreeVimager installer
echo:
echo **********************************************************************
echo ** Make NSIS FreeVimager installer
echo **********************************************************************
echo Please verify messages in opened window and then close it to continue.
"%NSISDIR%\makensisw.exe" FreeVimager.nsi

REM place all files into version folder
echo:
echo **********************************************************************
echo ** Place all files into %CURRENTVERSION%
echo **********************************************************************
move .\ContaCam-%CURRENTVERSION%-Setup.exe .\%CURRENTVERSION%\
move .\FreeVimager-%CURRENTVERSION%-Setup.exe .\%CURRENTVERSION%\

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
