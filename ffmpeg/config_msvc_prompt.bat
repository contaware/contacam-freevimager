@echo off

rem call Visual Studio Command Prompt
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
	pause
	goto :EOF
)

rem wait 2 seconds to let the user read the output from the above command
echo Wait that msys starts...
ping -n 3 127.0.0.1>nul

rem add inttypes.h and lame to environment variables
@set BATCHDIR=%~dp0
@set INCLUDE=%BATCHDIR%msvc;%INCLUDE%
@set LIB=%BATCHDIR%msvc;%LIB%

rem call msys.bat trying common places
if exist "c:\mingw\msys\1.0\msys.bat" (
	call "c:\mingw\msys\1.0\msys.bat"
) else if exist "d:\mingw\msys\1.0\msys.bat" (
	call "d:\mingw\msys\1.0\msys.bat"
) else if exist "e:\mingw\msys\1.0\msys.bat" (
	call "e:\mingw\msys\1.0\msys.bat"
) else (
	echo ERROR: Could not find the msys.bat file
	pause
)