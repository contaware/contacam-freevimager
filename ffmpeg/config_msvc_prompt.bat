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
echo Wait that msys2 starts...
ping -n 3 127.0.0.1>nul

rem set path to external include and lib (libx264)
@set BATCHDIR=%~dp0
@set INCLUDE=%BATCHDIR%msvc\include;%INCLUDE%
@set LIB=%BATCHDIR%msvc\lib;%LIB%

rem call msys2_shell.cmd trying common places
rem -use-full-path will inherit also the PATH environment variable
if exist "c:\msys64\msys2_shell.cmd" (
	call "c:\msys64\msys2_shell.cmd" -mingw32 -use-full-path
) else if exist "d:\msys64\msys2_shell.cmd" (
	call "d:\msys64\msys2_shell.cmd" -mingw32 -use-full-path
) else if exist "e:\msys64\msys2_shell.cmd" (
	call "e:\msys64\msys2_shell.cmd" -mingw32 -use-full-path
) else (
	echo ERROR: Could not find the msys2_shell.cmd file
	pause
)
